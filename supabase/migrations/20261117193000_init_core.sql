-- Kode Ecosystem core schema. Apply only to the dedicated Kode- Supabase project.
create extension if not exists pgcrypto;

create schema if not exists private;
revoke all on schema private from public, anon, authenticated;

create table public.profiles (
  id uuid primary key references auth.users(id) on delete cascade,
  display_name text,
  avatar_url text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

create table public.devices (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users(id) on delete cascade,
  device_name text not null,
  device_type text not null default 'kode_dot',
  device_identifier text,
  last_seen_at timestamptz,
  created_at timestamptz not null default now(),
  unique (user_id, device_identifier)
);
create index devices_user_id_idx on public.devices(user_id);

create table public.connector_providers (
  id uuid primary key default gen_random_uuid(),
  slug text not null unique,
  display_name text not null,
  category text not null,
  capability_manifest jsonb not null default '{}'::jsonb,
  status text not null default 'planned'
    check (status in ('planned','available','limited','unsupported','degraded')),
  documentation_url text,
  created_at timestamptz not null default now()
);

create table public.connected_accounts (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users(id) on delete cascade,
  provider_id uuid not null references public.connector_providers(id),
  external_account_id text,
  display_name text,
  connection_status text not null default 'pending'
    check (connection_status in ('pending','connected','reauth_required','revoked','error')),
  connected_at timestamptz,
  last_sync_at timestamptz,
  created_at timestamptz not null default now(),
  unique (user_id, provider_id, external_account_id)
);
create index connected_accounts_user_id_idx on public.connected_accounts(user_id);

create table public.connector_permissions (
  id uuid primary key default gen_random_uuid(),
  connected_account_id uuid not null references public.connected_accounts(id) on delete cascade,
  user_id uuid not null references auth.users(id) on delete cascade,
  scopes text[] not null default '{}',
  consented_at timestamptz,
  revoked_at timestamptz,
  created_at timestamptz not null default now()
);
create index connector_permissions_user_id_idx on public.connector_permissions(user_id);

create table public.applications (
  id uuid primary key default gen_random_uuid(),
  slug text not null unique,
  app_code text unique,
  name text not null,
  description text not null default '',
  category text not null,
  manifest jsonb not null default '{}'::jsonb,
  published boolean not null default false,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

create table public.installed_apps (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users(id) on delete cascade,
  application_id uuid not null references public.applications(id) on delete cascade,
  device_id uuid references public.devices(id) on delete set null,
  settings jsonb not null default '{}'::jsonb,
  installed_at timestamptz not null default now(),
  unique (user_id, application_id, device_id)
);
create index installed_apps_user_id_idx on public.installed_apps(user_id);

create table public.sync_cursors (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users(id) on delete cascade,
  connected_account_id uuid not null references public.connected_accounts(id) on delete cascade,
  resource_key text not null,
  cursor_value text,
  last_success_at timestamptz,
  updated_at timestamptz not null default now(),
  unique (connected_account_id, resource_key)
);
create index sync_cursors_user_id_idx on public.sync_cursors(user_id);

create table public.integration_events (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users(id) on delete cascade,
  provider_slug text not null,
  source_event_id text,
  event_type text not null,
  payload jsonb not null default '{}'::jsonb,
  source_timestamp timestamptz,
  retrieved_at timestamptz not null default now(),
  created_at timestamptz not null default now()
);
create index integration_events_user_created_idx on public.integration_events(user_id, created_at desc);

create table public.user_notifications (
  id uuid primary key default gen_random_uuid(),
  user_id uuid not null references auth.users(id) on delete cascade,
  title text not null,
  body text not null default '',
  category text not null default 'general',
  read_at timestamptz,
  created_at timestamptz not null default now()
);
create index user_notifications_user_created_idx on public.user_notifications(user_id, created_at desc);

create table public.audit_log (
  id bigint generated always as identity primary key,
  user_id uuid references auth.users(id) on delete set null,
  actor_type text not null default 'system',
  action text not null,
  resource_type text,
  resource_id text,
  metadata jsonb not null default '{}'::jsonb,
  created_at timestamptz not null default now()
);

-- Credential payloads are encrypted by the application before insertion.
-- This schema is not exposed through the Supabase Data API.
create table private.connector_secrets (
  connected_account_id uuid primary key references public.connected_accounts(id) on delete cascade,
  encrypted_payload bytea not null,
  key_version text not null,
  updated_at timestamptz not null default now()
);

alter table public.profiles enable row level security;
alter table public.devices enable row level security;
alter table public.connector_providers enable row level security;
alter table public.connected_accounts enable row level security;
alter table public.connector_permissions enable row level security;
alter table public.applications enable row level security;
alter table public.installed_apps enable row level security;
alter table public.sync_cursors enable row level security;
alter table public.integration_events enable row level security;
alter table public.user_notifications enable row level security;
alter table public.audit_log enable row level security;
alter table private.connector_secrets enable row level security;

create policy profiles_select_own on public.profiles for select to authenticated using ((select auth.uid()) = id);
create policy profiles_update_own on public.profiles for update to authenticated using ((select auth.uid()) = id) with check ((select auth.uid()) = id);

create policy devices_select_own on public.devices for select to authenticated using ((select auth.uid()) = user_id);
create policy devices_insert_own on public.devices for insert to authenticated with check ((select auth.uid()) = user_id);
create policy devices_update_own on public.devices for update to authenticated using ((select auth.uid()) = user_id) with check ((select auth.uid()) = user_id);
create policy devices_delete_own on public.devices for delete to authenticated using ((select auth.uid()) = user_id);

create policy connector_providers_read on public.connector_providers for select to authenticated using (true);
create policy connected_accounts_read_own on public.connected_accounts for select to authenticated using ((select auth.uid()) = user_id);
create policy connector_permissions_read_own on public.connector_permissions for select to authenticated using ((select auth.uid()) = user_id);

create policy applications_read_published on public.applications for select to authenticated using (published = true);
create policy installed_apps_select_own on public.installed_apps for select to authenticated using ((select auth.uid()) = user_id);
create policy installed_apps_insert_own on public.installed_apps for insert to authenticated with check ((select auth.uid()) = user_id);
create policy installed_apps_update_own on public.installed_apps for update to authenticated using ((select auth.uid()) = user_id) with check ((select auth.uid()) = user_id);
create policy installed_apps_delete_own on public.installed_apps for delete to authenticated using ((select auth.uid()) = user_id);

create policy integration_events_read_own on public.integration_events for select to authenticated using ((select auth.uid()) = user_id);
create policy notifications_read_own on public.user_notifications for select to authenticated using ((select auth.uid()) = user_id);
create policy notifications_update_own on public.user_notifications for update to authenticated using ((select auth.uid()) = user_id) with check ((select auth.uid()) = user_id);

-- Client grants are deliberately narrow. Connector credentials, sync cursors and audit data are server-only.
revoke all on public.profiles, public.devices, public.connector_providers,
  public.connected_accounts, public.connector_permissions, public.applications,
  public.installed_apps, public.sync_cursors, public.integration_events,
  public.user_notifications, public.audit_log from anon, authenticated;
grant select, update on public.profiles to authenticated;
grant select, insert, update, delete on public.devices to authenticated;
grant select on public.connector_providers, public.connected_accounts,
  public.connector_permissions, public.applications, public.integration_events to authenticated;
grant select, insert, update, delete on public.installed_apps to authenticated;
grant select, update on public.user_notifications to authenticated;
revoke all on all tables in schema private from public, anon, authenticated;
