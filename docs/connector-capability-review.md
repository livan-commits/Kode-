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

## Current implementation state

- Supabase provider rows: `planned`.
- OAuth client credentials: not configured.
- Provider-specific OAuth handlers: not deployed.
- Account dashboard API: deployed with JWT verification; read-only and scoped by the caller's JWT/RLS.
- No external player or smart-home data has been retrieved.
