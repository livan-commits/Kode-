import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP="https://kode-ecosystem.vercel.app/";
const CALLBACK="https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/steam-openid-callback";
const fail=(reason:string,status=400)=>Response.redirect(APP+"?steam=error&reason="+encodeURIComponent(reason),302);
const hex=(b:Uint8Array)=>[...b].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(v:string){return hex(new Uint8Array(await crypto.subtle.digest("SHA-256",new TextEncoder().encode(v))));}
Deno.serve(async(req:Request)=>{
 if(req.method!=="GET")return fail("method_not_allowed");
 const incoming=new URL(req.url),p=incoming.searchParams,state=p.get("state")||"";if(!/^[a-f0-9]{64}$/.test(state))return fail("invalid_state");
 const url=Deno.env.get("SUPABASE_URL"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY");if(!url||!service)return fail("backend_not_configured",503);
 const required=["openid.ns","openid.mode","openid.op_endpoint","openid.claimed_id","openid.identity","openid.return_to","openid.response_nonce","openid.assoc_handle","openid.signed","openid.sig"];
 if(required.some(k=>!p.get(k)))return fail("incomplete_openid_response");
 if(p.get("openid.ns")!=="http://specs.openid.net/auth/2.0"||p.get("openid.mode")!=="id_res")return fail("invalid_openid_response");
 const endpoint=p.get("openid.op_endpoint")||"";if(!["https://steamcommunity.com/openid/login","https://steamcommunity.com/openid/"].includes(endpoint))return fail("unexpected_identity_provider");
 if(p.get("openid.return_to")!==CALLBACK+"?state="+encodeURIComponent(state))return fail("return_url_mismatch");
 const claimed=p.get("openid.claimed_id")||"",identity=p.get("openid.identity")||"",match=claimed.match(/^https?:\/\/steamcommunity\.com\/openid\/id\/(\d{17})$/);
 if(!match||identity!==claimed)return fail("invalid_steam_identity");
 const signed=(p.get("openid.signed")||"").split(",");if(["op_endpoint","claimed_id","identity","return_to","response_nonce","assoc_handle"].some(k=>!signed.includes(k))||signed.some(k=>!p.has("openid."+k)))return fail("unsigned_openid_fields");
 const nonce=(p.get("openid.response_nonce")||"").match(/^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z)/),nonceTime=nonce?Date.parse(nonce[1]):NaN;
 if(!Number.isFinite(nonceTime)||nonceTime<Date.now()-600000||nonceTime>Date.now()+120000)return fail("stale_openid_nonce");
 const verify=new URLSearchParams();for(const [k,v] of p.entries())if(k.startsWith("openid."))verify.append(k,v);verify.set("openid.mode","check_authentication");
 let response:Response;try{response=await fetch("https://steamcommunity.com/openid/login",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:verify});}catch{return fail("steam_verification_unavailable",502);}
 const verification=await response.text();if(!response.ok||!/^is_valid:true\s*$/m.test(verification))return fail("steam_signature_invalid");
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:userId,error:consumeError}=await admin.rpc("consume_connector_auth_state",{p_state_hash:await sha256(state),p_provider_slug:"steam"});
 if(consumeError||!userId)return fail("state_replayed_or_expired");
 const {data:provider,error:providerError}=await admin.from("connector_providers").select("id").eq("slug","steam").maybeSingle();if(providerError||!provider)return fail("provider_not_configured",503);
 const steamId=match[1];
 const {error:upsertError}=await admin.from("connected_accounts").upsert({user_id:userId,provider_id:provider.id,external_account_id:steamId,display_name:"Steam · "+steamId,connection_status:"connected",connected_at:new Date().toISOString()},{onConflict:"user_id,provider_id,external_account_id"});
 if(upsertError){console.error("steam account link failed",upsertError.message);return fail("account_link_failed",503);}
 return Response.redirect(APP+"?steam=connected",302);
});
