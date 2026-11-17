-- Explicitly document the default-deny boundary for browser roles.
-- service_role remains the only role with table grants and bypasses RLS.
drop policy if exists connector_auth_states_deny_clients on private.connector_auth_states;
create policy connector_auth_states_deny_clients
on private.connector_auth_states
for all
to anon, authenticated
using (false)
with check (false);
