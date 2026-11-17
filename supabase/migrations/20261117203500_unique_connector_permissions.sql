-- One current permission snapshot per connected account.
-- OAuth reconnects update the current consent record instead of creating duplicates.
create unique index if not exists connector_permissions_account_unique
  on public.connector_permissions(connected_account_id);
