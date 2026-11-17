# Kode Ecosystem

Independent, modular companion platform for Kode Dot and connected services.

## Project status

This repository is the dedicated home for Kode Ecosystem. It is intentionally separate from `similateur-kd` and other existing applications.

## Architecture

- Kode Dot clients: local-first UI and device capabilities.
- Backend: Supabase Auth, Postgres with Row Level Security, Storage and Realtime.
- Server-side integrations: OAuth callbacks, webhooks and secret-bearing operations.
- Web portal: Vercel deployment, independent of the Kode Dot simulator.
- Connectors: isolated adapters with explicit capability states (`available`, `limited`, `unsupported`, `reauth_required`, `degraded`).

## Security principles

- Never store provider access tokens in firmware or browser storage.
- Keep OAuth secrets and refresh tokens server-side.
- Enforce per-user data access with Row Level Security.
- Record source and retrieval time for external data.
- Never claim a provider capability unless its API and required permissions have been verified.

## Web portal

The portal is deployed at https://kode-ecosystem.vercel.app. Its account area supports email/password sign-up, sign-in, password reset and a JWT-protected account dashboard. See [portal authentication setup and remaining checks](docs/portal-authentication.md). External provider connections remain planned until each provider has credentials, approved scopes and end-to-end tests.

## Next steps

See the [connector capability review](docs/connector-capability-review.md) for provider-specific API boundaries and activation criteria, plus the [security review](docs/security-review.md) for connected-account data retention and revocation. The `docs/` directory contains the architecture, data model and roadmap. Database migrations are kept in `supabase/migrations/` and must be applied only to a dedicated Kode Ecosystem Supabase project.
