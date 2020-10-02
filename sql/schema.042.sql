CREATE TABLE public.news (
    uuid character(36) NOT NULL,
    created bigint NOT NULL DEFAULT 0::bigint,
    body text NOT NULL
);

ALTER TABLE ONLY public.news ADD CONSTRAINT news_pkey PRIMARY KEY (uuid);
CREATE INDEX news_created ON public.news USING btree (created);

UPDATE public.versions SET value = 42 WHERE name = 'schema';
