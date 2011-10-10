alter table images add column is_viewed integer;
alter table images add column first_viewed_time integer;

update schema_version set version=1;
