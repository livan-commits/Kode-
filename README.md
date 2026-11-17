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

## Next steps

See the `docs/` directory for architecture, data model, security review and roadmap. Database migrations are kept in `supabase/migrations/` and must be applied only to a dedicated Kode Ecosystem Supabase project.
