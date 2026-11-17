alter table public.integration_events add column if not exists connected_account_id uuid references public.connected_accounts(id) on delete cascade;
create index if not exists integration_events_account_retrieved_idx on public.integration_events(connected_account_id, retrieved_at desc);
