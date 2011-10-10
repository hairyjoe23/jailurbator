create table images (
  reddit_author text,
  reddit_created integer,
  reddit_domain text,
  reddit_name text,
  reddit_link text,
  reddit_score integer,
  reddit_subreddit text,
  reddit_title text,
  reddit_thumbnail_url text,
  reddit_url text,

  saved_time integer,
  filename text,
  is_loved text,
  is_viewed text,
  first_viewed_time integer
);

create unique index images_idx_primary ON images (reddit_name, reddit_subreddit);
create index images_idx_saved_time ON images (saved_time);

create table schema_version (
  version integer NOT NULL
);

insert into schema_version (version) values (0);
