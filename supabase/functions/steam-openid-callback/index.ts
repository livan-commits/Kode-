import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";

const APP = "https://kode-ecosystem.vercel.app/";
const CALLBACK = "https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/steam-openid-callback";
const fail = (reason: string, status=400) => Response.redirect(APP + "?steam=error&reason=" + encodeURIComponent(reason), 302);
const hex = (bytes: Uint8Array) => [...bytes].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(value: string) { return hex(new Uint8Array(await crypto.subtle.digest("SHA-256", new TextEncoder().encode(value)))); }
Deno.serve(async (req: Request) => {
  if (req.method !== "GET") return fail("method_not_allowed");
  const incoming = new URL(req.url);
  const state = incoming.searchParams.get("state") || "";
  if (!/^[a-f0-9]{64}$/.test(state)) return fail("invalid_state");
  const url = Deno.env.get("SUPABASE_URL");
  const service = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY");
  if (!url || !service) return fail("backend_not_configured", 503);
  const admin = createClient(url, service, { auth: { persistSession: false, autoRefreshToken: false } });
  const stateHash = await sha256(state);
  const { data: pending, error: stateError } = await admin.schema("private").from("connector_auth_states")
    .select("user_id,provider_slug,expires_at,consumed_at").eq("state_hash", stateHash)
    .eq("provider_slug", "steam").gt("expires_at", new Date().toISOString()).is("consumed_at", null).maybeSingle();
  if (stateError || !pending) return fail("state_expired_or_used");
  const p = incoming.searchParams;
  const required = ["openid.ns","openid.mode","openid.op_endpoint","openid.claimed_id","openid.identity","openid.return_to","openid.response_nonce","openid.assoc_handle","openid.signed","openid.sig"];
  if (required.some(k=>!p.get(k))) return fail("incomplete_openid_response");
  if (p.get("openid.ns") !== "http://specs.openid.net/auth/2.0" || p.get("openid.mode") !== "id_res") return fail("invalid_openid_response");
  const endpoint = p.get("openid.op_endpoint") || "";
  if (!["https://steamcommunity.com/openid/login","https://steamcommunity.com/openid/"].includes(endpoint)) return fail("unexpected_identity_provider");
  const expectedReturn = CALLBACK + "?state=" + encodeURIComponent(state);
  if (p.get("openid.return_to") !== expectedReturn) return fail("return_url_mismatch");
  const claimed = p.get("openid.claimed_id") || "";
  const identity = p.get("openid.identity") || "";
  const match = claimed.match(/^https?:\/\/steamcommunity\.com\/openid\/id\/(\d{17})$/);
  if (!match || identity !== claimed) return fail("invalid_steam_identity");
  const signed = (p.get("openid.signed") || "").split(",");
  const signedRequired = ["op_endpoint","claimed_id","identity","return_to","response_nonce","assoc_handle"];
  if (signedRequired.some(k=>!signed.includes(k)) || signed.some(k=>!p.has("openid."+k))) return fail("unsigned_openid_fields");
  const nonce = p.get("openid.response_nonce") || "";
  const nonceMatch = nonce.match(/^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z)/);
  const nonceTime = nonceMatch ? Date.parse(nonceMatch[1]) : NaN;
  if (!Number.isFinite(nonceTime) || nonceTime < Date.now()-10*60*1000 || nonceTime > Date.now()+2*60*1000) return fail("stale_openid_nonce");
  const verify = new URLSearchParams();
  for (const [key,value] of p.entries()) if (key.startsWith("openid.")) verify.append(key,value);
  verify.set("openid.mode","check_authentication");
  let verification: Response;
  try { verification = await fetch("https://steamcommunity.com/openid/login", { method:"POST", headers:{"Content-Type":"application/x-www-form-urlencoded"}, body:verify }); }
  catch { return fail("steam_verification_unavailable", 502); }
  const verificationText = await verification.text();
  if (!verification.ok || !/^is_valid:true\s*$/m.test(verificationText)) return fail("steam_signature_invalid");
  const { data: consumed, error: consumeError } = await admin.schema("private").from("connector_auth_states")
    .update({ consumed_at: new Date().toISOString() }).eq("state_hash",stateHash)
    .is("consumed_at",null).gt("expires_at",new Date().toISOString()).select("user_id").maybeSingle();
  if (consumeError || !consumed) return fail("state_replayed_or_expired");
  const { data: provider, error: providerError } = await admin.from("connector_providers").select("id").eq("slug","steam").maybeSingle();
  if (providerError || !provider) return fail("provider_not_configured", 503);
  const steamId = match[1];
  const { error: upsertError } = await admin.from("connected_accounts").upsert({
    user_id: consumed.user_id, provider_id: provider.id, external_account_id: steamId,
    display_name: "Steam · " + steamId, connection_status: "connected", connected_at: new Date().toISOString()
  }, { onConflict:"user_id,provider_id,external_account_id" });
  if (upsertError) { console.error("steam account link failed",upsertError.message); return fail("account_link_failed",503); }
  return Response.redirect(APP + "?steam=connected", 302);
});
