import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const cors={"Access-Control-Allow-Origin":"*","Access-Control-Allow-Headers":"authorization, apikey, content-type, x-client-info","Access-Control-Allow-Methods":"GET, OPTIONS","Content-Type":"application/json; charset=utf-8"};
const json=(body:unknown,status=200)=>new Response(JSON.stringify(body),{status,headers:cors});
Deno.serve(async(req:Request)=>{
 if(req.method==="OPTIONS")return new Response("ok",{headers:cors});
 if(req.method!=="GET")return json({error:"method_not_allowed"},405);
 const authorization=req.headers.get("Authorization");
 if(!authorization?.startsWith("Bearer "))return json({error:"authentication_required"},401);
 const url=Deno.env.get("SUPABASE_URL");
 const key=Deno.env.get("SUPABASE_ANON_KEY")??Deno.env.get("SUPABASE_PUBLISHABLE_KEY");
 if(!url||!key)return json({error:"backend_not_configured"},503);
 const supabase=createClient(url,key,{global:{headers:{Authorization:authorization}},auth:{persistSession:false,autoRefreshToken:false}});
 const {data:authData,error:authError}=await supabase.auth.getUser();
 if(authError||!authData.user)return json({error:"invalid_session"},401);
 const userId=authData.user.id;
 const [profile,accounts,permissions,providers,applications,devices,installs,notifications,events]=await Promise.all([
  supabase.from("profiles").select("id,display_name,avatar_url,created_at,updated_at").eq("id",userId).maybeSingle(),
  supabase.from("connected_accounts").select("id,provider_id,external_account_id,display_name,connection_status,connected_at,last_sync_at,created_at").eq("user_id",userId).order("created_at",{ascending:false}),
  supabase.from("connector_permissions").select("id,connected_account_id,scopes,consented_at,revoked_at").eq("user_id",userId),
  supabase.from("connector_providers").select("id,slug,display_name,category,capability_manifest,status,documentation_url").order("display_name"),
  supabase.from("applications").select("id,slug,app_code,name,description,category,manifest").eq("published",true).order("category").order("name"),
  supabase.from("devices").select("id,device_name,device_type,device_identifier,last_seen_at,created_at").eq("user_id",userId).order("created_at",{ascending:false}),
  supabase.from("installed_apps").select("id,application_id,device_id,settings,installed_at").eq("user_id",userId).order("installed_at",{ascending:false}),
  supabase.from("user_notifications").select("id,title,body,category,read_at,created_at").eq("user_id",userId).order("created_at",{ascending:false}).limit(50),
  supabase.from("integration_events").select("id,provider_slug,event_type,payload,source_timestamp,retrieved_at").eq("user_id",userId).order("retrieved_at",{ascending:false}).limit(20)
 ]);
 const failed=[profile,accounts,permissions,providers,applications,devices,installs,notifications,events].find(x=>x.error);
 if(failed?.error){console.error("account-dashboard query failed",failed.error.message);return json({error:"data_unavailable"},503);}
 return json({user:{id:userId,email:authData.user.email??null},profile:profile.data,connected_accounts:accounts.data??[],permissions:permissions.data??[],providers:providers.data??[],applications:applications.data??[],devices:devices.data??[],installed_apps:installs.data??[],notifications:notifications.data??[],integration_events:events.data??[],generated_at:new Date().toISOString(),limitations:{unpublished_applications_hidden:true,provider_connection_requires_provider_specific_oauth_configuration:true,oauth_tokens_never_returned:true}});
});