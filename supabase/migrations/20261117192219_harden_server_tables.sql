-- Tighten explicit deny policies for server-only tables and index foreign keys.
create policy sync_cursors_deny_client
  on public.sync_cursors for all to anon, authenticated
  using (false) with check (false);
create policy audit_log_deny_client
  on public.audit_log for all to anon, authenticated
  using (false) with check (false);
create policy connector_secrets_deny_client
  on private.connector_secrets for all to anon, authenticated
  using (false) with check (false);

create index audit_log_user_id_idx on public.audit_log(user_id);
create index connected_accounts_provider_id_idx on public.connected_accounts(provider_id);
create index connector_permissions_connected_account_id_idx on public.connector_permissions(connected_account_id);
create index installed_apps_application_id_idx on public.installed_apps(application_id);
create index installed_apps_device_id_idx on public.installed_apps(device_id);
