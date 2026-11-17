create table if not exists private.connector_auth_states (
  state_hash text primary key,
  provider_slug text not null,
  user_id uuid not null references auth.users(id) on delete cascade,
  expires_at timestamptz not null,
  consumed_at timestamptz,
  created_at timestamptz not null default now()
);
create index if not exists connector_auth_states_expiry_idx
  on private.connector_auth_states(expires_at);
alter table private.connector_auth_states enable row level security;
revoke all on table private.connector_auth_states from public, anon, authenticated;
grant usage on schema private to service_role;
grant select, insert, update, delete on table private.connector_auth_states to service_role;
