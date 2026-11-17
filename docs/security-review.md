# Security baseline

- Apply least privilege to every provider OAuth scope.
- Use authorization code + PKCE where supported; validate state, redirect URI and issuer.
- Keep client secrets and refresh tokens in server-side secret storage, never in firmware, browser local storage, logs or analytics.
- Encrypt sensitive credentials at rest and rotate keys; support revocation and account disconnect.
- Enable RLS on every user-owned table and test cross-user isolation.
- Validate webhook signatures, timestamps and replay protection.
- Rate-limit authentication, OAuth callbacks and provider synchronization.
- Minimize collected data; expose consent, export and deletion flows.
- Record audit events without logging tokens or unnecessary personal data.
- Treat provider API capabilities as unverified until documented and tested.

This is a design baseline, not a claim that a production security audit has been completed.
## Connected-account data lifecycle

- Every newly ingested integration snapshot stores its `connected_account_id`, not only a provider slug and user ID.
- Disconnect marks the account revoked, revokes its recorded scopes, removes account sync cursors and encrypted credentials, and deletes snapshots linked to that account.
- Older snapshots created before account-level linkage are deleted only when the user has no other active account for that provider; this avoids deleting data belonging to a second linked account.
- Historical audit records are retained to document the revocation action. They must not contain provider tokens or raw credentials.
- The account-scoped event index supports bounded dashboard reads and targeted cleanup.
- This lifecycle has been implemented in the database function and migrations; a real-account deletion test remains part of end-to-end acceptance.

## OAuth state retention

OAuth state values are stored only as SHA-256 hashes, expire after ten minutes, and are consumed atomically once. State creation now opportunistically deletes expired or consumed rows older than one day so abandoned attempts do not accumulate indefinitely. The raw state is returned only to the browser for the provider redirect and is never stored in plaintext.
