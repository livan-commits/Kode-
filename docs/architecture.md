# Architecture

## Product boundaries

Kode Ecosystem is independent. It must not share deployment configuration, database projects, OAuth secrets or release pipelines with the Kode Dot simulator or unrelated applications.

## Components

1. Kode Dot client: local-first interface, device capabilities and cached read models.
2. Web portal: account management, connector consent, app catalogue and diagnostics.
3. API/backend: authorization checks, provider callbacks, webhooks, normalization and synchronization.
4. Supabase: Auth, Postgres, RLS, Storage and Realtime.
5. Connector adapters: provider-specific OAuth/API implementations behind a common contract.

## Data flow

Provider -> server-side connector -> normalized event -> authorization and validation -> database -> Realtime/read API -> user devices.

Firmware never receives provider client secrets or long-lived refresh tokens. Offline-capable apps read a local cache and reconcile changes when connectivity returns.

## Connector capability states

Every provider feature reports one of: available, limited, unsupported, reauth_required, or degraded. Include reason, scopes, last successful sync and source timestamp. Never fabricate unavailable data.

## Environments

Use isolated development, staging and production environments. Each has its own Supabase project, OAuth applications, callback URLs and Vercel project. Production secrets are never copied into preview builds.