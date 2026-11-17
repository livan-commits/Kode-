import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP="https://kode-ecosystem.vercel.app/";
const CALLBACK="https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/discord-oauth-callback";
const fail=(reason:string,status=400)=>Response.redirect(APP+"?discord=error&reason="+encodeURIComponent(reason),302);
const hex=(b:Uint8Array)=>[...b].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(v:string){return hex(new Uint8Array(await crypto.subtle.digest("SHA-256",new TextEncoder().encode(v))));}
Deno.serve(async(req:Request)=>{
 if(req.method!=="GET")return fail("method_not_allowed",405);
 const p=new URL(req.url).searchParams,state=p.get("state")||"",code=p.get("code")||"";
 if(!/^[a-f0-9]{64}$/.test(state))return fail("invalid_state");
 const url=Deno.env.get("SUPABASE_URL"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY"),clientId=Deno.env.get("DISCORD_CLIENT_ID"),clientSecret=Deno.env.get("DISCORD_CLIENT_SECRET");
 if(!url||!service||!clientId||!clientSecret)return fail("discord_oauth_not_configured",503);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:userId,error:consumeError}=await admin.rpc("consume_connector_auth_state",{p_state_hash:await sha256(state),p_provider_slug:"discord"});
 if(consumeError||!userId)return fail("state_replayed_or_expired");
 if(p.has("error"))return fail(p.get("error")==="access_denied"?"consent_denied":"authorization_failed");
 if(!code||code.length>2048)return fail("missing_authorization_code");
 let tokenResponse:Response;
 try{tokenResponse=await fetch("https://discord.com/api/v10/oauth2/token",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded","Accept":"application/json"},body:new URLSearchParams({client_id:clientId,client_secret:clientSecret,code,grant_type:"authorization_code",redirect_uri:CALLBACK})});}
 catch{return fail("discord_token_exchange_unavailable",502);}
 const tokens=await tokenResponse.json().catch(()=>({})) as {access_token?:string;token_type?:string;scope?:string;error?:string};
 if(!tokenResponse.ok||typeof tokens.access_token!=="string")return fail("discord_token_exchange_failed",502);
 const headers={Authorization:"Bearer "+tokens.access_token,Accept:"application/json"};
 let userResponse:Response,connectionsResponse:Response;
 try{[userResponse,connectionsResponse]=await Promise.all([fetch("https://discord.com/api/v10/users/@me",{headers}),fetch("https://discord.com/api/v10/users/@me/connections",{headers})]);}
 catch{return fail("discord_api_unavailable",502);}
 const user=await userResponse.json().catch(()=>({})) as {id?:string;username?:string;global_name?:string|null;avatar?:string|null;discriminator?:string};
 const connections=await connectionsResponse.json().catch(()=>[]);
 if(!userResponse.ok||typeof user.id!=="string"||!/^\d{17,20}$/.test(user.id))return fail("discord_identity_invalid",502);
 if(!connectionsResponse.ok||!Array.isArray(connections))return fail("discord_connections_unavailable",502);
 const {data:provider,error:providerError}=await admin.from("connector_providers").select("id").eq("slug","discord").maybeSingle();
 if(providerError||!provider)return fail("provider_not_configured",503);
 const displayName=user.global_name||user.username||"Compte Discord";
 const {data:account,error:upsertError}=await admin.from("connected_accounts").upsert({user_id:userId,provider_id:provider.id,external_account_id:user.id,display_name:displayName,connection_status:"connected",connected_at:new Date().toISOString()},{onConflict:"user_id,provider_id,external_account_id"}).select("id").single();
 if(upsertError||!account){console.error("Discord account link failed",upsertError?.message);return fail("account_link_failed",503);}
 const scopes=(tokens.scope||"identify connections").split(" ").filter(Boolean);
 const {error:permissionError}=await admin.from("connector_permissions").upsert({connected_account_id:account.id,user_id:userId,scopes,consented_at:new Date().toISOString(),revoked_at:null},{onConflict:"connected_account_id"});
 if(permissionError){console.error("Discord permission record failed",permissionError.message);return fail("permission_record_failed",503);}
 const retrievedAt=new Date().toISOString();
 const safeConnections=connections.slice(0,100).map((c:any)=>({id:typeof c.id==="string"?c.id:null,type:typeof c.type==="string"?c.type:null,name:typeof c.name==="string"?c.name:null,verified:c.verified===true,visibility:typeof c.visibility==="number"?c.visibility:null,friend_sync:c.friend_sync===true,show_activity:c.show_activity===true,two_way_link:c.two_way_link===true}));
 const events=[
  {user_id:userId,provider_slug:"discord",event_type:"discord.profile.snapshot",payload:{id:user.id,username:user.username||null,global_name:user.global_name||null,avatar:user.avatar||null,discriminator:user.discriminator||null,scopes},source_timestamp:retrievedAt,retrieved_at:retrievedAt},
  {user_id:userId,provider_slug:"discord",event_type:"discord.connections.snapshot",payload:{count:connections.length,returned_count:safeConnections.length,connections:safeConnections},source_timestamp:retrievedAt,retrieved_at:retrievedAt}
 ];
 const {error:eventError}=await admin.from("integration_events").insert(events);
 if(eventError){console.error("Discord snapshot persistence failed",eventError.message);return fail("snapshot_persistence_failed",503);}
 // Tokens are discarded. Presence, friends, messages and activity scopes are not requested.
 return Response.redirect(APP+"?discord=connected",302);
});
