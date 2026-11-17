import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP_ORIGIN="https://kode-ecosystem.vercel.app";
const CALLBACK="https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/steam-openid-callback";
const cors={"Access-Control-Allow-Origin":APP_ORIGIN,"Access-Control-Allow-Headers":"authorization, apikey, content-type, x-client-info","Access-Control-Allow-Methods":"POST, OPTIONS","Vary":"Origin"};
const json=(body:unknown,status=200)=>new Response(JSON.stringify(body),{status,headers:{...cors,"Content-Type":"application/json"}});
const hex=(b:Uint8Array)=>[...b].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(v:string){return hex(new Uint8Array(await crypto.subtle.digest("SHA-256",new TextEncoder().encode(v))));}
Deno.serve(async(req:Request)=>{
 if(req.method==="OPTIONS")return new Response("ok",{headers:cors});
 if(req.method!=="POST")return json({error:"method_not_allowed"},405);
 const auth=req.headers.get("Authorization")||"";if(!auth.startsWith("Bearer "))return json({error:"authentication_required"},401);
 const url=Deno.env.get("SUPABASE_URL"),anon=Deno.env.get("SUPABASE_ANON_KEY")||Deno.env.get("SUPABASE_PUBLISHABLE_KEY"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY");
 if(!url||!anon||!service)return json({error:"backend_not_configured"},503);
 const userClient=createClient(url,anon,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:userData,error:userError}=await userClient.auth.getUser(auth.slice(7));if(userError||!userData.user)return json({error:"invalid_session"},401);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const random=new Uint8Array(32);crypto.getRandomValues(random);const state=hex(random),stateHash=await sha256(state);
 const {error}=await admin.rpc("create_connector_auth_state",{p_state_hash:stateHash,p_provider_slug:"steam",p_user_id:userData.user.id,p_expires_at:new Date(Date.now()+600000).toISOString()});
 if(error){console.error("steam state creation failed",error.message);return json({error:"connection_initialization_failed"},503);}
 const returnTo=CALLBACK+"?state="+encodeURIComponent(state);
 const params=new URLSearchParams({"openid.ns":"http://specs.openid.net/auth/2.0","openid.mode":"checkid_setup","openid.return_to":returnTo,"openid.realm":APP_ORIGIN+"/","openid.identity":"http://specs.openid.net/auth/2.0/identifier_select","openid.claimed_id":"http://specs.openid.net/auth/2.0/identifier_select"});
 return json({authorization_url:"https://steamcommunity.com/openid/login?"+params.toString()});
});
