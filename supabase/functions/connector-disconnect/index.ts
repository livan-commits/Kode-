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
 const {data:userData,error:userError}=await userClient.auth.getUser(auth.slice(7));
 if(userError||!userData.user)return json({error:"invalid_session"},401);
 let body:{connected_account_id?:string};try{body=await req.json();}catch{return json({error:"invalid_request"},400);}
 const accountId=body.connected_account_id;
 if(typeof accountId!=="string"||!/^[0-9a-f-]{36}$/i.test(accountId))return json({error:"invalid_account"},400);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data,error}=await admin.rpc("revoke_connected_account",{p_connected_account_id:accountId,p_user_id:userData.user.id});
 if(error){console.error("connector revocation failed",error.message);return json({error:"disconnect_failed"},503);}
 if(data!==true)return json({error:"account_not_found"},404);
 return json({disconnected:true,connected_account_id:accountId});
});
