# Connector framework

Each connector is an isolated adapter implementing the shared contract in packages/contracts/src/connector.ts.

A connector must declare provider identity, supported features, required scopes, authentication mode, rate-limit behavior and data provenance. Each feature reports a capability state and a user-readable explanation.

Implementation order is evidence-driven: verify official API documentation, developer registration requirements, permitted use, endpoints, scopes, quotas and webhook support before coding. If an API does not expose a requested field, mark it unsupported; do not scrape private endpoints or imply account-wide access.

OAuth credentials and refresh tokens are handled only by trusted server-side code. Disconnect must revoke tokens where supported and erase stored credentials.