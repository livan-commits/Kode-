import { createClient } from "https://cdn.jsdelivr.net/npm/@supabase/supabase-js@2.119.0/+esm";
const URL="https://jcfhmqpuuiscnvzmzlie.supabase.co";
const KEY="sb_publishable_OHryNGW_5q4Z1SpOrpLm8w_w-8uKR_X";
const db=createClient(URL,KEY), $=s=>document.querySelector(s);
const categoryNames={core:"Kode Core",gaming:"Gaming",life:"Vie quotidienne",smart_home:"Maison connectée",maker:"Maker",ai:"IA",control:"Contrôle",play:"Jeux",security:"Sécurité"};
const esc=v=>String(v??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
let apps=[],providers=[],activeCategory="",busy=false,recoveryMode=false;
function msg(s,error=false){$("#auth-message").textContent=s;$("#auth-message").classList.toggle("error",error);}
function dashMsg(s,error=false){$("#dashboard-status").textContent=s;$("#dashboard-status").classList.toggle("error",error);}
function renderApps(){const term=$("#search").value.trim().toLowerCase(),cat=activeCategory||$("#category").value;const list=apps.filter(a=>(!cat||a.category===cat)&&(!term||[a.name,a.code,a.slug,a.description,a.category].join(" ").toLowerCase().includes(term)));$("#shown-count").textContent=list.length+" / "+apps.length;$("#apps").innerHTML=list.length?list.map(a=>'<article class="app-card"><div class="card-top"><span class="app-code">'+esc(a.code)+'</span><span class="state">Planifiée</span></div><h3>'+esc(a.name)+'</h3><p>'+esc(categoryNames[a.category]||a.category)+'</p></article>').join(""):'<div class="loading">Aucune application ne correspond à votre recherche.</div>';}
function renderProviders(){$("#providers").innerHTML=providers.map(p=>'<article class="provider-card"><b>'+esc(p.display_name)+'</b><span>À étudier</span></article>').join("");}
function initCats(){const cats=[...new Set(apps.map(a=>a.category))];$("#category").innerHTML='<option value="">Toutes les catégories</option>'+cats.map(c=>'<option value="'+esc(c)+'">'+esc(categoryNames[c]||c)+'</option>').join("");$("#categories").innerHTML='<button class="pill active" data-cat="">Tout</button>'+cats.map(c=>'<button class="pill" data-cat="'+esc(c)+'">'+esc(categoryNames[c]||c)+'</button>').join("");$("#categories").addEventListener("click",e=>{const b=e.target.closest("button[data-cat]");if(!b)return;activeCategory=b.dataset.cat;$("#category").value=activeCategory;document.querySelectorAll(".pill").forEach(x=>x.classList.toggle("active",x===b));renderApps();});}
async function loadCatalog(){try{const [a,p]=await Promise.all([fetch("/applications/catalog.json"),fetch("/connectors/providers.json")]);if(!a.ok||!p.ok)throw Error();const [aa,pp]=await Promise.all([a.json(),p.json()]);apps=aa.applications;providers=pp.providers;$("#app-total").textContent=apps.length+" applications référencées";$("#provider-total").textContent=providers.length+" services référencés";initCats();renderApps();renderProviders();}catch{$("#apps").innerHTML='<div class="loading">Impossible de charger le catalogue. Réessayez plus tard.</div>';$("#providers").innerHTML="";}}
function renderDashboard(d){const pm=new Map((d.providers||[]).map(p=>[p.id,p]));const cards=(rows,fn,empty)=>rows.length?rows.map(fn).join(""):'<p class="subtle">'+empty+'</p>';const item=(title,state,detail)=>'<div class="dashboard-item"><b>'+esc(title)+'</b><span class="mini-state">'+esc(state||"")+'</span><small>'+esc(detail||"")+'</small></div>';const counts=[["Services associés",(d.connected_accounts||[]).length],["Appareils",(d.devices||[]).length],["Applications installées",(d.installed_apps||[]).length],["Notifications",(d.notifications||[]).length]];$("#dashboard-content").innerHTML='<div class="dashboard-stats">'+counts.map(([n,v])=>'<div class="stat-card"><strong>'+v+'</strong><span>'+n+'</span></div>').join("")+'</div><section class="dashboard-block"><h4>Services associés</h4><div class="dashboard-list">'+cards(d.connected_accounts||[],a=>item(pm.get(a.provider_id)?.display_name||"Service",a.connection_status,a.display_name||a.external_account_id||"Compte associé")+(pm.get(a.provider_id)?.slug==="steam"&&a.connection_status==="connected"?'<button type="button" class="sync-button" data-steam-sync="'+esc(a.id)+'">Synchroniser Steam</button>':"")+'<button type="button" class="disconnect-button" data-disconnect-account="'+esc(a.id)+'">Dissocier</button>',"Aucun compte externe associé.")+'</div></section><section class="dashboard-block"><h4>Données synchronisées</h4><div class="dashboard-list">'+(()=>{const ev=d.integration_events||[],p=ev.find(x=>x.event_type==="steam.profile.snapshot"),l=ev.find(x=>x.event_type==="steam.library.snapshot"),g=ev.find(x=>x.event_type==="google.profile.snapshot"),dp=ev.find(x=>x.event_type==="discord.profile.snapshot"),dc=ev.find(x=>x.event_type==="discord.connections.snapshot"),tp=ev.find(x=>x.event_type==="twitch.profile.snapshot");if(!p&&!l&&!g&&!dp&&!dc&&!tp)return '<p class="subtle">Aucune donnée externe synchronisée pour le moment.</p>';const profile=p?.payload||{},library=l?.payload||{},google=g?.payload||{},discord=dp?.payload||{},linked=dc?.payload||{};return (p?'<div class="dashboard-item"><b>Steam · '+esc(profile.personaname||"Profil")+'</b><span class="mini-state">Profil</span><small>Mis à jour : '+esc(new Date(p.retrieved_at).toLocaleString("fr-FR"))+'</small></div>':"")+(l?'<div class="dashboard-item"><b>Bibliothèque Steam</b><span class="mini-state">'+esc(library.total_count||0)+' jeux</span><small>'+esc(library.returned_count||0)+' jeux enregistrés dans le dernier relevé</small></div>':"")+(g?'<div class="dashboard-item"><b>Google · '+esc(google.name||google.email||"Profil")+'</b><span class="mini-state">Identité</span><small>'+esc(google.email||"E-mail non partagé")+'</small></div>':"")+(dp?'<div class="dashboard-item"><b>Discord · '+esc(discord.global_name||discord.username||"Profil")+'</b><span class="mini-state">Identité</span><small>'+esc(linked.count||0)+' comptes liés détectés</small></div>':"")+(dc?'<div class="dashboard-item"><b>Comptes associés à Discord</b><span class="mini-state">'+esc(linked.count||0)+' comptes</span><small>'+esc(linked.returned_count||0)+' enregistrés</small></div>':"")+(tp?'<div class="dashboard-item"><b>Twitch · '+esc(tp.payload.display_name||tp.payload.login||"Profil")+'</b><span class="mini-state">Identité</span><small>@'+esc(tp.payload.login||"")+'</small></div>':"");})()+'</div></section><section class="dashboard-block"><h4>Mes appareils</h4><div class="dashboard-list">'+cards(d.devices||[],x=>item(x.device_name,x.device_type,x.device_identifier||"Identifiant non renseigné"),"Aucun appareil enregistré.")+'</div></section><section class="dashboard-block"><h4>Notifications récentes</h4><div class="dashboard-list">'+cards((d.notifications||[]).slice(0,5),x=>item(x.title,x.category,x.body),"Aucune notification.")+'</div></section>';const t=new Date(d.generated_at);$("#user-since").textContent="Données actualisées : "+(Number.isNaN(t.getTime())?"—":t.toLocaleString("fr-FR"));}
async function showDashboard(session){$("#auth-panel").hidden=true;$("#user-panel").hidden=false;$("#auth-state").textContent="Connecté";$("#user-email").textContent=session.user.email||"Compte Kode";dashMsg("Récupération sécurisée de vos données…");try{const r=await fetch(URL+"/functions/v1/account-dashboard",{headers:{Authorization:"Bearer "+session.access_token,apikey:KEY}});const d=await r.json().catch(()=>({}));if(!r.ok)throw Error(d.error==="backend_not_configured"?"Le backend n'est pas configuré.":d.error==="data_unavailable"?"Les données du compte sont momentanément indisponibles.":"Tableau de bord indisponible ("+r.status+").");renderDashboard(d);dashMsg("Espace synchronisé. Les connexions externes restent soumises à leur configuration et à leurs autorisations.");}catch(e){$("#dashboard-content").innerHTML="";dashMsg(e.message||"Erreur de chargement.",true);}}
function signedOut(){$("#auth-panel").hidden=false;$("#recovery-panel").hidden=true;$("#user-panel").hidden=true;$("#auth-state").textContent="Non connecté";$("#dashboard-content").innerHTML="";}
function showRecovery(){recoveryMode=true;$("#auth-panel").hidden=true;$("#user-panel").hidden=true;$("#recovery-panel").hidden=false;$("#auth-state").textContent="Réinitialisation du mot de passe";$("#new-password").focus();}
$("#recovery-form").addEventListener("submit",async e=>{e.preventDefault();const password=$("#new-password").value,confirm=$("#confirm-password").value;if(password.length<8){$("#recovery-message").textContent="Le mot de passe doit contenir au moins 8 caractères.";return;}if(password!==confirm){$("#recovery-message").textContent="Les deux mots de passe ne correspondent pas.";return;}const button=$("#recovery-submit");button.disabled=true;try{const {error}=await db.auth.updateUser({password});if(error)throw error;recoveryMode=false;$("#recovery-panel").hidden=true;$("#new-password").value="";$("#confirm-password").value="";$("#auth-state").textContent="Mot de passe modifié";$("#auth-message").textContent="Votre mot de passe a été modifié. Vous pouvez continuer à utiliser votre compte.";const {data:{session}}=await db.auth.getSession();if(session)await showDashboard(session);else signedOut();
}catch(error){$("#recovery-message").textContent=error.message||"Modification impossible. Demandez un nouveau lien."; }finally{button.disabled=false;}});
$("#auth-form").addEventListener("submit",async e=>{e.preventDefault();if(busy)return;busy=true;$("#auth-submit").disabled=true;try{const {data,error}=await db.auth.signInWithPassword({email:$("#auth-email").value.trim(),password:$("#auth-password").value});if(error)throw error;await showDashboard(data.session);msg("Connexion réussie.");}catch(e){msg(e.message||"Connexion impossible.",true);}finally{busy=false;$("#auth-submit").disabled=false;}});
$("#auth-signup").addEventListener("click",async()=>{if(busy)return;const email=$("#auth-email").value.trim(),password=$("#auth-password").value;if(!email||password.length<8){msg("Saisissez un e-mail et un mot de passe de 8 caractères minimum.",true);return;}busy=true;$("#auth-signup").disabled=true;try{const {data,error}=await db.auth.signUp({email,password,options:{emailRedirectTo:location.origin+"/"}});if(error)throw error;if(data.session){await showDashboard(data.session);msg("Compte créé et connecté.");}else msg("Inscription enregistrée. Vérifiez votre e-mail pour confirmer votre adresse avant de vous connecter.");}catch(e){msg(e.message||"Création du compte impossible.",true);}finally{busy=false;$("#auth-signup").disabled=false;}});
$("#auth-reset").addEventListener("click",async()=>{const email=$("#auth-email").value.trim();if(!email){msg("Saisissez d'abord votre adresse e-mail.",true);return;}try{const {error}=await db.auth.resetPasswordForEmail(email,{redirectTo:location.origin+"/"});if(error)throw error;msg("Si cette adresse correspond à un compte, un e-mail de réinitialisation va être envoyé.");}catch(e){msg(e.message||"Réinitialisation impossible.",true);}});
$("#dashboard-content").addEventListener("click",async event=>{
 const sync=event.target.closest("button[data-steam-sync]");
 if(sync){
  const accountId=sync.dataset.steamSync;sync.disabled=true;sync.textContent="Synchronisation…";
  try{const {data:{session}}=await db.auth.getSession();if(!session)throw Error("Session expirée.");
   const response=await fetch(URL+"/functions/v1/steam-sync",{method:"POST",headers:{Authorization:"Bearer "+session.access_token,apikey:KEY,"Content-Type":"application/json"},body:JSON.stringify({connected_account_id:accountId})});
   const result=await response.json().catch(()=>({}));
   if(!response.ok)throw Error(result.error==="steam_api_key_required"?"La clé Steam Web API n'est pas encore configurée côté serveur.":result.error==="steam_api_key_invalid"?"La clé Steam Web API configurée est invalide.":result.message||"Synchronisation Steam indisponible.");
   await showDashboard(session);dashMsg("Steam synchronisé : "+result.library.total_count+" jeux détectés.");
  }catch(error){dashMsg(error.message||"Synchronisation Steam impossible.",true);sync.disabled=false;sync.textContent="Synchroniser Steam";}
  return;
 }
 const button=event.target.closest("button[data-disconnect-account]");if(!button)return;
 const accountId=button.dataset.disconnectAccount;if(!accountId)return;
 if(!confirm("Dissocier ce compte externe de votre compte Kode ?"))return;
 button.disabled=true;
 try{const {data:{session}}=await db.auth.getSession();if(!session)throw Error("Session expirée.");
 const response=await fetch(URL+"/functions/v1/connector-disconnect",{method:"POST",headers:{Authorization:"Bearer "+session.access_token,apikey:KEY,"Content-Type":"application/json"},body:JSON.stringify({connected_account_id:accountId})});
 const result=await response.json().catch(()=>({}));if(!response.ok)throw Error("Impossible de dissocier ce service.");
 await showDashboard(session);dashMsg("Service dissocié.");
 }catch(error){dashMsg(error.message||"Erreur lors de la dissociation.",true);button.disabled=false;}
});
$("#connect-twitch").addEventListener("click",async()=>{
 const button=$("#connect-twitch");button.disabled=true;
 try{
  const {data:{session}}=await db.auth.getSession();if(!session)throw Error("Reconnectez-vous avant de lier Twitch.");
  const response=await fetch(URL+"/functions/v1/twitch-oauth-start",{method:"POST",headers:{Authorization:"Bearer "+session.access_token,apikey:KEY,"Content-Type":"application/json"},body:"{}"});
  const result=await response.json().catch(()=>({}));
  if(!response.ok||typeof result.authorization_url!=="string")throw Error(result.error==="twitch_oauth_not_configured"?"La connexion Twitch attend encore la configuration des identifiants OAuth côté serveur.":"La liaison Twitch n'est pas disponible pour le moment.");
  const target=new URL(result.authorization_url);if(target.origin!=="https://id.twitch.tv"||target.pathname!=="/oauth2/authorize")throw Error("Adresse d'autorisation Twitch invalide.");
  location.assign(target.href);
 }catch(error){dashMsg(error.message||"Impossible de démarrer la liaison Twitch.",true);button.disabled=false;}
});
$("#connect-discord").addEventListener("click",async()=>{
 const button=$("#connect-discord");button.disabled=true;
 try{
  const {data:{session}}=await db.auth.getSession();if(!session)throw Error("Reconnectez-vous avant de lier Discord.");
  const response=await fetch(URL+"/functions/v1/discord-oauth-start",{method:"POST",headers:{Authorization:"Bearer "+session.access_token,apikey:KEY,"Content-Type":"application/json"},body:"{}"});
  const result=await response.json().catch(()=>({}));
  if(!response.ok||typeof result.authorization_url!=="string")throw Error(result.error==="discord_oauth_not_configured"?"La connexion Discord attend encore la configuration des identifiants OAuth côté serveur.":"La liaison Discord n'est pas disponible pour le moment.");
  const target=new URL(result.authorization_url);if(target.origin!=="https://discord.com"||target.pathname!=="/oauth2/authorize")throw Error("Adresse d'autorisation Discord invalide.");
  location.assign(target.href);
 }catch(error){dashMsg(error.message||"Impossible de démarrer la liaison Discord.",true);button.disabled=false;}
});
$("#connect-google").addEventListener("click",async()=>{
 const button=$("#connect-google");button.disabled=true;
 try{
  const {data:{session}}=await db.auth.getSession();if(!session)throw Error("Reconnectez-vous avant de lier Google.");
  const response=await fetch(URL+"/functions/v1/google-oauth-start",{method:"POST",headers:{Authorization:"Bearer "+session.access_token,apikey:KEY,"Content-Type":"application/json"},body:"{}"});
  const result=await response.json().catch(()=>({}));
  if(!response.ok||typeof result.authorization_url!=="string")throw Error(result.error==="google_oauth_not_configured"?"La connexion Google attend encore la configuration des identifiants OAuth côté serveur.":"La liaison Google n'est pas disponible pour le moment.");
  const target=new URL(result.authorization_url);if(target.origin!=="https://accounts.google.com"||target.pathname!=="/o/oauth2/v2/auth")throw Error("Adresse d'autorisation Google invalide.");
  location.assign(target.href);
 }catch(error){dashMsg(error.message||"Impossible de démarrer la liaison Google.",true);button.disabled=false;}
});
$("#connect-steam").addEventListener("click",async()=>{
 const button=$("#connect-steam");button.disabled=true;
 try{
  const {data:{session}}=await db.auth.getSession();if(!session)throw Error("Reconnectez-vous avant de lier Steam.");
  const response=await fetch(URL+"/functions/v1/steam-openid-start",{method:"POST",headers:{Authorization:"Bearer "+session.access_token,apikey:KEY,"Content-Type":"application/json"},body:"{}"});
  const result=await response.json().catch(()=>({}));
  if(!response.ok||typeof result.authorization_url!=="string")throw Error("La liaison Steam n'est pas disponible pour le moment.");
  const target=new URL(result.authorization_url);if(target.origin!=="https://steamcommunity.com"||target.pathname!=="/openid/login")throw Error("Adresse d'autorisation Steam invalide.");
  location.assign(target.href);
 }catch(error){dashMsg(error.message||"Impossible de démarrer la liaison Steam.",true);button.disabled=false;}
});
$("#auth-signout").addEventListener("click",async()=>{const {error}=await db.auth.signOut();if(error)dashMsg(error.message,true);else{signedOut();msg("Vous êtes déconnecté.");}});
$("#search").addEventListener("input",renderApps);$("#category").addEventListener("change",()=>{activeCategory="";document.querySelectorAll(".pill").forEach((x,i)=>x.classList.toggle("active",i===0));renderApps();});
loadCatalog();
db.auth.onAuthStateChange((event,session)=>{if(event==="PASSWORD_RECOVERY"){showRecovery();return;}if(recoveryMode)return;if(session){$("#auth-panel").hidden=true;$("#recovery-panel").hidden=true;$("#user-panel").hidden=false;$("#auth-state").textContent="Connecté";$("#user-email").textContent=session.user.email||"Compte Kode";}else signedOut();});
const {data:{session}}=await db.auth.getSession();if(session)await showDashboard(session);else signedOut();

const callbackUrl=new URL(location.href),steamResult=callbackUrl.searchParams.get("steam");
if(steamResult==="connected"){dashMsg("Compte Steam lié. Seule l'identité Steam est confirmée pour le moment.");history.replaceState(null,"",location.pathname);}
else if(steamResult==="error"){dashMsg("La liaison Steam n'a pas abouti. Vérifiez votre compte et réessayez.",true);history.replaceState(null,"",location.pathname);}
const googleResult=callbackUrl.searchParams.get("google");
if(googleResult==="connected"){dashMsg("Compte Google lié. Seule l'identité et le profil de base sont enregistrés.");history.replaceState(null,"",location.pathname);}
else if(googleResult==="error"){dashMsg("La liaison Google n'a pas abouti. Vérifiez la configuration OAuth et réessayez.",true);history.replaceState(null,"",location.pathname);}
const twitchResult=callbackUrl.searchParams.get("twitch");
if(twitchResult==="connected"){dashMsg("Compte Twitch lié. Seule l’identité et le profil public de base sont enregistrés.");history.replaceState(null,"",location.pathname);}
else if(twitchResult==="error"){dashMsg("La liaison Twitch n’a pas abouti. Vérifiez la configuration OAuth et réessayez.",true);history.replaceState(null,"",location.pathname);}
const discordResult=callbackUrl.searchParams.get("discord");
if(discordResult==="connected"){dashMsg("Compte Discord lié. Profil et comptes associés récupérés selon les autorisations accordées.");history.replaceState(null,"",location.pathname);}
else if(discordResult==="error"){dashMsg("La liaison Discord n'a pas abouti. Vérifiez la configuration OAuth et réessayez.",true);history.replaceState(null,"",location.pathname);}

