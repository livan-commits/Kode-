import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";
const APP="https://kode-ecosystem.vercel.app";
const cors={"Access-Control-Allow-Origin":APP,"Access-Control-Allow-Headers":"authorization, apikey, content-type, x-client-info","Access-Control-Allow-Methods":"POST, OPTIONS","Vary":"Origin"};
const json=(body:unknown,status=200)=>new Response(JSON.stringify(body),{status,headers:{...cors,"Content-Type":"application/json"}});
async function steamGet(path:string,key:string){
 const response=await fetch("https://api.steampowered.com/"+path+"&key="+encodeURIComponent(key),{headers:{"Accept":"application/json"}});
 if(!response.ok)throw new Error("steam_api_"+response.status);
 return await response.json();
}
Deno.serve(async(req:Request)=>{
 if(req.method==="OPTIONS")return new Response("ok",{headers:cors});
 if(req.method!=="POST")return json({error:"method_not_allowed"},405);
 const authorization=req.headers.get("Authorization")||"";
 if(!authorization.startsWith("Bearer "))return json({error:"authentication_required"},401);
 const url=Deno.env.get("SUPABASE_URL"),anon=Deno.env.get("SUPABASE_ANON_KEY")||Deno.env.get("SUPABASE_PUBLISHABLE_KEY"),service=Deno.env.get("SUPABASE_SERVICE_ROLE_KEY"),steamKey=Deno.env.get("STEAM_WEB_API_KEY");
 if(!url||!anon||!service)return json({error:"backend_not_configured"},503);
 const userClient=createClient(url,anon,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:authData,error:authError}=await userClient.auth.getUser(authorization.slice(7));
 if(authError||!authData.user)return json({error:"invalid_session"},401);
 let body:{connected_account_id?:string};try{body=await req.json();}catch{return json({error:"invalid_request"},400);}
 const accountId=body.connected_account_id;
 if(typeof accountId!=="string"||!/^[0-9a-f-]{36}$/i.test(accountId))return json({error:"invalid_account"},400);
 const admin=createClient(url,service,{auth:{persistSession:false,autoRefreshToken:false}});
 const {data:provider,error:providerError}=await admin.from("connector_providers").select("id").eq("slug","steam").maybeSingle();
 if(providerError||!provider)return json({error:"provider_not_configured"},503);
 const {data:account,error:accountError}=await admin.from("connected_accounts").select("id,external_account_id,connection_status").eq("id",accountId).eq("user_id",authData.user.id).eq("provider_id",provider.id).maybeSingle();
 if(accountError)return json({error:"account_lookup_failed"},503);
 if(!account||account.connection_status!=="connected")return json({error:"steam_account_not_connected"},404);
 if(!/^\d{17}$/.test(account.external_account_id||""))return json({error:"invalid_steam_id"},400);
 if(!steamKey)return json({error:"steam_api_key_required",message:"La clé Steam Web API doit être configurée côté serveur sous STEAM_WEB_API_KEY."},503);
 let player:any,games:any;
 try{
  const [profileResponse,gamesResponse]=await Promise.all([
   steamGet("ISteamUser/GetPlayerSummaries/v2/?steamids="+account.external_account_id,steamKey),
   steamGet("IPlayerService/GetOwnedGames/v1/?steamid="+account.external_account_id+"&include_appinfo=1&include_played_free_games=1",steamKey)
  ]);
  player=profileResponse?.response?.players?.[0]??null;
  games=gamesResponse?.response??null;
 }catch(error){
  const message=error instanceof Error?error.message:"unknown";
  console.error("Steam API sync failed",message);
  return json({error:message==="steam_api_401"?"steam_api_key_invalid":message==="steam_api_403"?"steam_api_forbidden":"steam_api_unavailable"},502);
 }
 if(!player)return json({error:"steam_profile_unavailable",message:"Le profil Steam n'est pas accessible avec les paramètres de confidentialité actuels."},403);
 const retrievedAt=new Date().toISOString();
 const profilePayload={steamid:player.steamid,personaname:player.personaname,profileurl:player.profileurl,avatar:player.avatar,avatarmedium:player.avatarmedium,avatarfull:player.avatarfull,personastate:player.personastate,communityvisibilitystate:player.communityvisibilitystate,profilestate:player.profilestate};
 const gameList=Array.isArray(games?.games)?games.games:[];
 const libraryPayload={total_count:Number(games?.game_count??gameList.length),returned_count:Math.min(gameList.length,500),games:gameList.slice(0,500).map((g:any)=>({appid:g.appid,name:g.name,playtime_forever:g.playtime_forever,playtime_2weeks:g.playtime_2weeks,img_icon_url:g.img_icon_url,img_logo_url:g.img_logo_url}))};
 const rows=[
  {user_id:authData.user.id,provider_slug:"steam",event_type:"steam.profile.snapshot",payload:profilePayload,source_timestamp:retrievedAt,retrieved_at:retrievedAt},
  {user_id:authData.user.id,provider_slug:"steam",event_type:"steam.library.snapshot",payload:libraryPayload,source_timestamp:retrievedAt,retrieved_at:retrievedAt}
 ];
 const {error:insertError}=await admin.from("integration_events").insert(rows);
 if(insertError){console.error("Steam snapshot persistence failed",insertError.message);return json({error:"snapshot_persistence_failed"},503);}
 await admin.from("connected_accounts").update({last_sync_at:retrievedAt}).eq("id",account.id).eq("user_id",authData.user.id);
 return json({synced:true,retrieved_at:retrievedAt,profile:{name:profilePayload.personaname,avatar:profilePayload.avatarfull,profile_url:profilePayload.profileurl},library:{total_count:libraryPayload.total_count,stored_count:libraryPayload.returned_count,visibility_limited:libraryPayload.total_count===0}});
});
