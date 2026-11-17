create or replace function public.revoke_connected_account(
  p_connected_account_id uuid,
  p_user_id uuid
) returns boolean
language plpgsql
security definer
set search_path = ''
as $$
declare
  v_provider_slug text;
begin
  select cp.slug into v_provider_slug
    from public.connected_accounts ca
    join public.connector_providers cp on cp.id = ca.provider_id
   where ca.id = p_connected_account_id
     and ca.user_id = p_user_id
   for update of ca;

  if v_provider_slug is null then
    return false;
  end if;

  update public.connected_accounts
     set connection_status = 'revoked'
   where id = p_connected_account_id
     and user_id = p_user_id;

  update public.connector_permissions
     set revoked_at = now()
   where connected_account_id = p_connected_account_id
     and user_id = p_user_id
     and revoked_at is null;

  delete from public.sync_cursors
   where connected_account_id = p_connected_account_id
     and user_id = p_user_id;

  delete from private.connector_secrets
   where connected_account_id = p_connected_account_id;

  insert into public.audit_log(user_id,actor_type,action,resource_type,resource_id,metadata)
  values(p_user_id,'user','connector.disconnected','connector_provider',v_provider_slug,
    jsonb_build_object('connected_account_id',p_connected_account_id));

  return true;
end;
$$;

revoke all on function public.revoke_connected_account(uuid,uuid) from public,anon,authenticated;
grant execute on function public.revoke_connected_account(uuid,uuid) to service_role;
