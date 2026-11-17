import "jsr:@supabase/functions-js/edge-runtime.d.ts";
import { createClient } from "npm:@supabase/supabase-js@2";

const APP_ORIGIN = "https://kode-ecosystem.vercel.app";
const CALLBACK = "https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/steam-openid-callback";
const cors = { "Access-Control-Allow-Origin": APP_ORIGIN, "Access-Control-Allow-Headers": "authorization, apikey, content-type, x-client-info", "Access-Control-Allow-Methods": "POST, OPTIONS", "Vary": "Origin" };
const json = (body: unknown, status=200) => new Response(JSON.stringify(body), { status, headers: { ...cors, "Content-Type": "application/json" } });
const hex = (bytes: Uint8Array) => [...bytes].map(x=>x.toString(16).padStart(2,"0")).join("");
async function sha256(value: string) { return hex(new Uint8Array(await crypto.subtle.digest("SHA-256", new TextEncoder().encode(value)))); }
Deno.serve(async (req: Request) => {
  if (req.method === "OPTIONS") return new Response("ok", { headers: cors });
  if (req.method !== "POST") return json({ error: "method_not_allowed" }, 405);
  const auth = req.headers.get("Authorization") || "";
  if (!auth.startsWith("Bearer ")) return json({ error: "authentication_required" }, 401);
  const url = Deno.env.get("SUPABASE_URL");
  const anon = Deno.env.get("SUPABASE_ANON_KEY") || Deno.env.get("SUPABASE_PUBLISHABLE_KEY");
  const service = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY");
  if (!url || !anon || !service) return json({ error: "backend_not_configured" }, 503);
  const token = auth.slice(7);
  const userClient = createClient(url, anon, { auth: { persistSession: false, autoRefreshToken: false } });
  const { data: userData, error: userError } = await userClient.auth.getUser(token);
  if (userError || !userData.user) return json({ error: "invalid_session" }, 401);
  const admin = createClient(url, service, { auth: { persistSession: false, autoRefreshToken: false } });
  const random = new Uint8Array(32); crypto.getRandomValues(random);
  const state = hex(random);
  const stateHash = await sha256(state);
  const { error: insertError } = await admin.schema("private").from("connector_auth_states").insert({
    state_hash: stateHash, provider_slug: "steam", user_id: userData.user.id,
    expires_at: new Date(Date.now() + 10 * 60 * 1000).toISOString()
  });
  if (insertError) { console.error("steam state insert failed", insertError.message); return json({ error: "connection_initialization_failed" }, 503); }
  const returnTo = CALLBACK + "?state=" + encodeURIComponent(state);
  const params = new URLSearchParams({
    "openid.ns": "http://specs.openid.net/auth/2.0",
    "openid.mode": "checkid_setup",
    "openid.return_to": returnTo,
    "openid.realm": APP_ORIGIN + "/",
    "openid.identity": "http://specs.openid.net/auth/2.0/identifier_select",
    "openid.claimed_id": "http://specs.openid.net/auth/2.0/identifier_select"
  });
  return json({ authorization_url: "https://steamcommunity.com/openid/login?" + params.toString() });
});
