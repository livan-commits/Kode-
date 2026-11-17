# Connector capability review — 2026-11-17

This is an engineering assessment, not a claim that any provider is connected. The database and public registry remain `planned` until a provider-specific implementation has credentials, consent, tests and an operational revocation path.

## Priority order

| Provider | What official interfaces support | Main constraint | Decision |
|---|---|---|---|
| Steam | Public Web API methods; protected Steamworks methods can expose player stats/achievements for a game | Some methods require a Web API key and game-specific context; visibility/privacy and publisher permissions apply | First gaming connector candidate |
| Google | OAuth 2.0 web-server authorization; API access is controlled by individually requested scopes | Each Google product API must be enabled; sensitive/restricted scopes may require verification | First general account connector candidate |
| Discord | OAuth2 identity and user connections via the `identify` and `connections` scopes | Access to friends, activity and richer presence is restricted or approval-dependent; not a universal game activity API | Basic profile/connections only initially |
| Home Assistant | REST API for state reads and service calls; WebSocket API for ongoing updates | User must provide an instance URL and create/authorize a token; network reachability matters | First smart-home connector candidate |
| Xbox | Xbox Services includes profiles, friends/presence, stats, leaderboards and achievements via Microsoft game-development APIs | XSAPI / GDK is designed for title developers; this does not establish a public consumer OAuth API for a third-party companion | Research/partner access required |
| PlayStation Network | Sony developer portal exists, but a public, supported consumer profile/trophy API suitable for this app has not been established from official docs | Community reverse-engineered APIs and NPSSO flows are not an acceptable production integration basis | Block until Sony-supported access is confirmed |
| Epic Games | Developer APIs exist | Product-specific access and partner/app approval must be assessed before user-data sync | Research required |
| Spotify | Web API exists | App-mode, quota and extended access rules must be assessed | Research required |

## Provider implementation notes

### Steam
Official Steamworks documentation distinguishes public Web API methods from protected methods intended for trusted backend applications. Its user-stats interface documents `GetPlayerAchievements`, which requires an API key, SteamID and AppID. Do not infer that every user's complete game history or all achievements are readable: check game-specific availability and profile privacy.

References:
- https://partner.steamgames.com/doc/webapi_overview
- https://partner.steamgames.com/doc/webapi/isteamuserstats

### Google
Use server-side OAuth authorization-code flow with exact HTTPS redirect URI matching, least-privilege scopes, incremental consent and offline access only when a background sync feature genuinely needs it. Keep client secrets and refresh tokens server-side.

References:
- https://developers.google.com/identity/protocols/oauth2/web-server
- https://developers.google.com/identity/protocols/oauth2

### Discord
Start with `identify` and optionally `connections` only. The official user API documents `GET /users/@me/connections` as requiring the `connections` scope. Do not promise a live activity feed, friends list or arbitrary game telemetry; those capabilities are not included in basic identity/connection consent.

References:
- https://discord.com/developers/docs/topics/oauth2
- https://discord.com/developers/docs/resources/user#get-user-connections

### Home Assistant
Use the documented REST API for one-off reads/actions and WebSocket API for event-driven updates. Authentication uses a Bearer token. A long-lived access token is manually created by the user in their Home Assistant profile; it is not an OAuth login and must be treated like a password. Ask for the instance URL and token only through a secure connection setup screen, then encrypt the token before persistence.

References:
- https://developers.home-assistant.io/docs/api/rest/
- https://www.home-assistant.io/docs/authentication/

### Xbox
Microsoft documents Xbox Services features for player profile, friends/presence, stats, leaderboards and achievements. The official overview recommends client-side Xbox Services APIs (XSAPI) and describes REST/websocket services as the underlying platform. This is developer-oriented access, not proof that an arbitrary third-party service can obtain consumer account data with a normal OAuth app. Do not implement scraping or reuse undocumented endpoints.

References:
- https://learn.microsoft.com/en-us/gaming/gdk/docs/services/fundamentals/live-xbl-overview
- https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/live/rest/uri/atoc-xboxlivews-reference-uris

### PlayStation Network
The Sony developer portal is the official starting point. Community-maintained PSN trophy libraries describe reverse-engineered endpoints and browser/session-derived tokens; these are not official API guarantees. Do not request, store or automate extraction of NPSSO/session credentials. Revisit only if Sony grants documented access or a supported partner route is confirmed.

References:
- https://developer.sony.com/
- https://github.com/andshrew/PlayStation-Trophies

## Security and acceptance checklist

A provider may move from `planned` to `limited` or `available` only after:
1. Official API terms, scopes, approval requirements and quotas are recorded.
2. A provider-specific authorization flow uses state, PKCE where supported, exact redirect allowlists and CSRF protection.
3. Tokens are encrypted at rest server-side and never returned to a browser, Dot client or logs.
4. The user can inspect and revoke granted scopes and disconnect the account.
5. Tests cover consent denial, invalid/expired token, refresh, revocation, rate limiting, privacy restrictions and provider outages.
6. Every synced record stores source, retrieval timestamp and provider event timestamp where available.
7. No undocumented endpoint, credential scraping or account-password collection is used.


## Steam identity-linking implementation

The project now contains a first server-side Steam account-linking path using the official Steamworks-documented OpenID 2.0 browser flow:

- `steam-openid-start` requires a valid Kode Ecosystem user JWT and creates a random, short-lived, hashed state.
- `steam-openid-callback` verifies the OpenID response with Steam's `check_authentication` endpoint, validates the claimed SteamID and nonce, and consumes the state atomically before linking the account.
- The state table is private; only the server-side service role can call narrowly scoped state RPC functions.
- The browser receives only an authorization URL; Steam identity and state verification happen server-side.

**Current capability boundary:** this links a Steam identity / SteamID only. It does not yet fetch games, achievements, playtime, presence or ownership. Those require separate Steam Web API methods, user privacy settings and a server-side Steam Web API key. The provider remains `planned` until the full callback is exercised with a real Steam account and the deployed integration passes end-to-end security tests.

Official reference: https://partner.steamgames.com/doc/features/auth

## Current implementation state

- Supabase provider rows: `planned`.
- OAuth client credentials: not configured.
- Provider-specific OAuth handlers: not deployed.
- Account dashboard API: deployed with JWT verification; read-only and scoped by the caller's JWT/RLS.
- No external player or smart-home data has been retrieved.


## Steam profile and library sync

A user-authenticated `steam-sync` Edge Function is now deployed. It:
- verifies the Kode account JWT and the linked Steam account ownership;
- requires the server-side `STEAM_WEB_API_KEY` secret (never sent to the browser);
- retrieves the Steam player summary and owned-games library through Steam Web API;
- stores timestamped snapshots in `integration_events`, capped at 500 game records per snapshot while retaining the reported total;
- updates the account's last-sync timestamp.

The account dashboard exposes a manual “Synchroniser Steam” action and summarizes the latest profile/library snapshots. A missing API key returns an explicit configuration error; it does not fabricate data. Private Steam profiles or hidden libraries may yield limited/empty results. Achievements and per-game statistics are not included yet.

**Activation requirement:** set the Supabase Edge Function secret `STEAM_WEB_API_KEY` for project `jcfhmqpuuiscnvzmzlie` using Supabase Dashboard → Edge Functions → Secrets. The key must be kept server-side. Then the user can link Steam and trigger synchronization. Until the secret is configured and a real account is tested, Steam data sync remains limited/unverified.

Official API references:
- https://partner.steamgames.com/doc/webapi_overview
- https://partner.steamgames.com/doc/webapi/isteamuser
- https://partner.steamgames.com/doc/webapi/iplayerservice


## Google identity connector

A server-side Google OAuth authorization-code flow is implemented and deployed:
- authenticated start endpoint creates a random, hashed, one-time state with a 10-minute expiry;
- callback consumes the state once, exchanges the authorization code server-to-server, and retrieves the OpenID Connect user profile;
- only `openid email profile` are requested; access is not requested to Gmail, Drive, Calendar, or other Google APIs;
- Google subject, basic display identity, approved scopes, and a timestamped profile snapshot are associated with the user's Kode account;
- Google access tokens are deliberately discarded and are not persisted.

Required Supabase Edge Function secrets:
- `GOOGLE_OAUTH_CLIENT_ID`
- `GOOGLE_OAUTH_CLIENT_SECRET`

Required Google Cloud OAuth client redirect URI (must match exactly):
`https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/google-oauth-callback`

The Google OAuth consent screen and OAuth web client must be configured by the project owner. Until those secrets and the exact redirect URI are configured and an end-to-end account test succeeds, the Google connector remains **limited / not verified**. No Gmail, Drive, Calendar or background synchronization is claimed.

Official references:
- https://developers.google.com/identity/protocols/oauth2/web-server
- https://developers.google.com/identity/openid-connect/openid-connect


## Discord identity and linked-account connector

A Discord OAuth2 authorization-code flow is implemented and deployed:
- one-time hashed state with a 10-minute expiry and authenticated flow initiation;
- OAuth scopes requested: `identify connections`;
- callback retrieves the Discord user profile and the user's linked third-party accounts through `GET /users/@me/connections`;
- profile and a bounded snapshot of up to 100 linked connections are stored with retrieval timestamps;
- access and refresh tokens are discarded and are not stored.

Required Supabase Edge Function secrets:
- `DISCORD_CLIENT_ID`
- `DISCORD_CLIENT_SECRET`

Required Discord Developer Portal redirect URI:
`https://jcfhmqpuuiscnvzmzlie.supabase.co/functions/v1/discord-oauth-callback`

This does **not** provide Discord friends, messages, live presence, or activity tracking. Those are separate scopes/features, and some require Discord approval. The connector remains unverified until application credentials are configured and a real account completes the flow.

Official references:
- https://discord.com/developers/docs/topics/oauth2
- https://discord.com/developers/docs/resources/user#get-current-user-connections
