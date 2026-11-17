# Account Dashboard API

Supabase Edge Function: `account-dashboard`

## Request
`GET https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/account-dashboard`

Headers:
- `Authorization: Bearer <Supabase access token>`
- `apikey: <Supabase publishable key>`

The function validates the caller with Supabase Auth and uses that caller's JWT for every database query. Row Level Security remains active.

## Response
Returns the authenticated user's profile, connected-account metadata, granted scopes, provider registry, published applications, registered devices, installed apps and latest notifications.

OAuth tokens, encrypted connector secrets and service-role credentials are never returned. Only published applications are exposed through this API. Providers in planned state are not considered connected or available.

## Deployment
JWT verification must remain enabled. Runtime environment must provide `SUPABASE_URL` and either `SUPABASE_ANON_KEY` or `SUPABASE_PUBLISHABLE_KEY`.
