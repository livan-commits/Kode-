import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP="https://kode-ecosystem.vercel.app";
const cors={"Access-Control-Allow-Origin":APP,"Access-Control-Allow-Headers":"authorization, apikey, content-type, x-client-info","Access-Control-Allow-Methods":"POST, OPTIONS","Vary":"Origin"};
const json=(body:unknown,status=200)=>new Response(JSON.stringify(body),{status,headers:{...cors,"Content-Type":"application/json"}});
Deno.serve(async(req:Request)=>{
 if(req.method==="OPTIONS")return new Response("ok",{headers:cors});
 if(req.method!=="POST")return json({error:"method_not_allowed"},405);
 const auth=req.headers.get("Authorization")||"";if(!auth.startsWith("Bearer "))return json({error:"authentication_required"},401);
 const url=Deno.env.get("SUPABASE_URL"),anon=Deno.env.get("SUPABASE_ANON_KEY")||Deno.env.get("SUPABASE_PUBLISHABLE_KEY"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY");
 if(!url||!anon||!service)return json({error:"backend_not_configured"},503);
 const userClient=createClient(url,anon,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:userData,error:userError}=await userClient.auth.getUser(auth.slice(7));if(userError||!userData.user)return json({error:"invalid_session"},401);
 let body:{provider_slug?:string};try{body=await req.json();}catch{return json({error:"invalid_request"},400);}
 const slug=body.provider_slug;if(typeof slug!=="string"||!/^[a-z0-9-]{2,64}$/.test(slug))return json({error:"invalid_provider"},400);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:provider,error:providerError}=await admin.from("connector_providers").select("id").eq("slug",slug).maybeSingle();
 if(providerError||!provider)return json({error:"provider_not_found"},404);
 const {data:account,error:accountError}=await admin.from("connected_accounts").select("id").eq("user_id",userData.user.id).eq("provider_id",provider.id).maybeSingle();
 if(accountError)return json({error:"account_lookup_failed"},503);
 if(!account)return json({error:"account_not_connected"},404);
 const {error:updateError}=await admin.from("connected_accounts").update({connection_status:"revoked"}).eq("id",account.id).eq("user_id",userData.user.id);
 if(updateError){console.error("connector disconnect failed",updateError.message);return json({error:"disconnect_failed"},503);}
 await admin.from("audit_log").insert({user_id:userData.user.id,actor_type:"user",action:"connector.disconnected",resource_type:"connector_provider",resource_id:slug,metadata:{connected_account_id:account.id}});
 return json({disconnected:true,provider_slug:slug});
});
