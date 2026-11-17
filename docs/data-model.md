# Core data model

The initial schema separates identity, devices, applications, connector configuration, synchronization and audit history.

- profiles: application profile keyed to the authenticated user.
- devices: user-owned Kode Dot or companion clients.
- connector_providers: public provider catalogue and supported capability metadata.
- connected_accounts: server-managed provider account links; secrets are not stored in client-readable rows.
- connector_permissions: granted scopes and consent state.
- sync_cursors: server-managed incremental synchronization checkpoints.
- integration_events: normalized, provenance-bearing external events.
- user_notifications: user-facing notifications.
- audit_log: security-relevant actions and administrative traces.

All user-owned rows require RLS policies. Server-only tables must not grant direct client writes. External records retain provider, source identifier, retrieval time and source timestamp where available.