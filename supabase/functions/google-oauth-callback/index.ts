import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP="https://kode-ecosystem.vercel.app/";
const CALLBACK="https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/google-oauth-callback";
const fail=(reason:string,status=400)=>Response.redirect(APP+"?google=error&reason="+encodeURIComponent(reason),302);
const hex=(b:Uint8Array)=>[...b].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(v:string){return hex(new Uint8Array(await crypto.subtle.digest("SHA-256",new TextEncoder().encode(v))));}
Deno.serve(async(req:Request)=>{
 if(req.method!=="GET")return fail("method_not_allowed",405);
 const p=new URL(req.url).searchParams,state=p.get("state")||"",code=p.get("code")||"";
 if(!/^[a-f0-9]{64}$/.test(state))return fail("invalid_state");
 const url=Deno.env.get("SUPABASE_URL"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY"),clientId=Deno.env.get("GOOGLE_OAUTH_CLIENT_ID"),clientSecret=Deno.env.get("GOOGLE_OAUTH_CLIENT_SECRET");
 if(!url||!service||!clientId||!clientSecret)return fail("google_oauth_not_configured",503);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:userId,error:consumeError}=await admin.rpc("consume_connector_auth_state",{p_state_hash:await sha256(state),p_provider_slug:"google"});
 if(consumeError||!userId)return fail("state_replayed_or_expired");
 if(p.has("error"))return fail(p.get("error")==="access_denied"?"consent_denied":"authorization_failed");
 if(!code||code.length>2048)return fail("missing_authorization_code");
 let tokenResponse:Response;
 try{tokenResponse=await fetch("https://oauth2.googleapis.com/token",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded","Accept":"application/json"},body:new URLSearchParams({code,client_id:clientId,client_secret:clientSecret,redirect_uri:CALLBACK,grant_type:"authorization_code"})});}
 catch{return fail("google_token_exchange_unavailable",502);}
 const tokens=await tokenResponse.json().catch(()=>({})) as {access_token?:string;token_type?:string;scope?:string;error?:string};
 if(!tokenResponse.ok||typeof tokens.access_token!=="string")return fail(tokens.error==="invalid_grant"?"authorization_code_invalid":"google_token_exchange_failed",502);
 let profileResponse:Response;
 try{profileResponse=await fetch("https://openidconnect.googleapis.com/v1/userinfo",{headers:{Authorization:"Bearer "+tokens.access_token,Accept:"application/json"}});}
 catch{return fail("google_profile_unavailable",502);}
 const profile=await profileResponse.json().catch(()=>({})) as {sub?:string;name?:string;email?:string;picture?:string;email_verified?:boolean};
 if(!profileResponse.ok||typeof profile.sub!=="string"||!/^[A-Za-z0-9._-]{1,255}$/.test(profile.sub))return fail("google_identity_invalid",502);
 const {data:provider,error:providerError}=await admin.from("connector_providers").select("id").eq("slug","google").maybeSingle();
 if(providerError||!provider)return fail("provider_not_configured",503);
 const displayName=profile.name||profile.email||"Compte Google";
 const {data:account,error:upsertError}=await admin.from("connected_accounts").upsert({user_id:userId,provider_id:provider.id,external_account_id:profile.sub,display_name:displayName,connection_status:"connected",connected_at:new Date().toISOString()},{onConflict:"user_id,provider_id,external_account_id"}).select("id").single();
 if(upsertError||!account){console.error("Google account link failed",upsertError?.message);return fail("account_link_failed",503);}
 const scopes=(tokens.scope||"openid email profile").split(" ").filter(Boolean);
 const {error:permissionError}=await admin.from("connector_permissions").insert({connected_account_id:account.id,user_id:userId,scopes,consented_at:new Date().toISOString()});
 if(permissionError)console.error("Google permission audit insert failed",permissionError.message);
 await admin.from("integration_events").insert({user_id:userId,provider_slug:"google",event_type:"google.profile.snapshot",payload:{sub:profile.sub,name:profile.name||null,email:profile.email||null,email_verified:profile.email_verified===true,picture:profile.picture||null,scopes},source_timestamp:new Date().toISOString()});
 // Access tokens are intentionally discarded: this first phase links identity only and requests no offline access.
 return Response.redirect(APP+"?google=connected",302);
});
