create table images (
  reddit_author TEXT,
  reddit_created INTEGER,
  reddit_domain TEXT,
  reddit_name TEXT,
  reddit_link TEXT,
  reddit_score INTEGER,
  reddit_subreddit TEXT,
  reddit_title TEXT,
  reddit_thumbnail_url TEXT,
  reddit_url TEXT,

  saved_time INTEGER,
  filename TEXT,
  is_loved INTEGER
);

create unique index images_idx_primary ON images (reddit_name, reddit_subreddit);
create index images_idx_saved_time ON images (saved_time);

create table schema_version (
  version INTEGER NOT NULL
);

insert into schema_version (version) values (0);
