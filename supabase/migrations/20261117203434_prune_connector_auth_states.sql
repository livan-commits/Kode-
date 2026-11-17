create or replace function public.create_connector_auth_state(
  p_state_hash text,
  p_provider_slug text,
  p_user_id uuid,
  p_expires_at timestamptz
) returns void
language plpgsql
security definer
set search_path = ''
as $$
begin
  delete from private.connector_auth_states
   where expires_at < now() - interval '1 day'
      or (consumed_at is not null and consumed_at < now() - interval '1 day');

  insert into private.connector_auth_states(state_hash,provider_slug,user_id,expires_at)
  values(p_state_hash,p_provider_slug,p_user_id,p_expires_at);
end;
$$;

revoke all on function public.create_connector_auth_state(text,text,uuid,timestamptz) from public,anon,authenticated;
grant execute on function public.create_connector_auth_state(text,text,uuid,timestamptz) to service_role;
