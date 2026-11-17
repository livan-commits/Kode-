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