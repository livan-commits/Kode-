import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP="https://kode-ecosystem.vercel.app/";
const CALLBACK="https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/twitch-oauth-callback";
const fail=(reason:string,status=400)=>Response.redirect(APP+"?twitch=error&reason="+encodeURIComponent(reason),302);
const hex=(b:Uint8Array)=>[...b].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(v:string){return hex(new Uint8Array(await crypto.subtle.digest("SHA-256",new TextEncoder().encode(v))));}
Deno.serve(async(req:Request)=>{
 if(req.method!=="GET")return fail("method_not_allowed",405);
 const p=new URL(req.url).searchParams,state=p.get("state")||"",code=p.get("code")||"";
 if(!/^[a-f0-9]{64}$/.test(state))return fail("invalid_state");
 const url=Deno.env.get("SUPABASE_URL"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY"),clientId=Deno.env.get("TWITCH_CLIENT_ID"),clientSecret=Deno.env.get("TWITCH_CLIENT_SECRET");
 if(!url||!service||!clientId||!clientSecret)return fail("twitch_oauth_not_configured",503);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:userId,error:consumeError}=await admin.rpc("consume_connector_auth_state",{p_state_hash:await sha256(state),p_provider_slug:"twitch"});
 if(consumeError||!userId)return fail("state_replayed_or_expired");
 if(p.has("error"))return fail(p.get("error")==="access_denied"?"consent_denied":"authorization_failed");
 if(!code||code.length>2048)return fail("missing_authorization_code");
 let tokenResponse:Response;
 try{tokenResponse=await fetch("https://id.twitch.tv/oauth2/token",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded","Accept":"application/json"},body:new URLSearchParams({client_id:clientId,client_secret:clientSecret,code,grant_type:"authorization_code",redirect_uri:CALLBACK})});}
 catch{return fail("twitch_token_exchange_unavailable",502);}
 const tokens=await tokenResponse.json().catch(()=>({})) as {access_token?:string;refresh_token?:string;scope?:string[];expires_in?:number};
 if(!tokenResponse.ok||typeof tokens.access_token!=="string")return fail("twitch_token_exchange_failed",502);
 let validationResponse:Response;
 try{validationResponse=await fetch("https://id.twitch.tv/oauth2/validate",{headers:{Authorization:"OAuth "+tokens.access_token,Accept:"application/json"}});}
 catch{return fail("twitch_token_validation_unavailable",502);}
 const validation=await validationResponse.json().catch(()=>({})) as {client_id?:string;user_id?:string;scopes?:string[];expires_in?:number};
 if(!validationResponse.ok||validation.client_id!==clientId||typeof validation.user_id!=="string"||!/^\d{1,20}$/.test(validation.user_id))return fail("twitch_token_validation_failed",502);
 let profileResponse:Response;
 try{profileResponse=await fetch("https://api.twitch.tv/helix/users",{headers:{"Client-Id":clientId,Authorization:"Bearer "+tokens.access_token,Accept:"application/json"}});}
 catch{return fail("twitch_profile_unavailable",502);}
 const profile=await profileResponse.json().catch(()=>({})) as {data?:Array<{id?:string,login?:string,display_name?:string,profile_image_url?:string,description?:string}>};
 const user=profile.data?.[0];
 if(!profileResponse.ok||!user||user.id!==validation.user_id)return fail("twitch_identity_invalid",502);
 const {data:provider,error:providerError}=await admin.from("connector_providers").select("id").eq("slug","twitch").maybeSingle();
 if(providerError||!provider)return fail("provider_not_configured",503);
 const displayName=user.display_name||user.login||"Compte Twitch";
 const {data:account,error:upsertError}=await admin.from("connected_accounts").upsert({user_id:userId,provider_id:provider.id,external_account_id:user.id,display_name:displayName,connection_status:"connected",connected_at:new Date().toISOString()},{onConflict:"user_id,provider_id,external_account_id"}).select("id").single();
 if(upsertError||!account){console.error("Twitch account link failed",upsertError?.message);return fail("account_link_failed",503);}
 const scopes=Array.isArray(validation.scopes)?validation.scopes:[];
 const {error:permissionError}=await admin.from("connector_permissions").upsert({connected_account_id:account.id,user_id:userId,scopes,consented_at:new Date().toISOString(),revoked_at:null},{onConflict:"connected_account_id"});
 if(permissionError){console.error("Twitch permission record failed",permissionError.message);return fail("permission_record_failed",503);}
 const retrievedAt=new Date().toISOString();
 const {error:eventError}=await admin.from("integration_events").insert({user_id:userId,connected_account_id:account.id,provider_slug:"twitch",event_type:"twitch.profile.snapshot",payload:{id:user.id,login:user.login||null,display_name:user.display_name||null,profile_image_url:user.profile_image_url||null,description:user.description||null,scopes},source_timestamp:retrievedAt,retrieved_at:retrievedAt});
 if(eventError){console.error("Twitch profile snapshot persistence failed",eventError.message);return fail("snapshot_persistence_failed",503);}
 // Access and refresh tokens are deliberately discarded. This phase links identity only.
 return Response.redirect(APP+"?twitch=connected",302);
});
