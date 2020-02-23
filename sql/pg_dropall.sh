#!/bin/bash

printf "Enter password: "
read passwd
export PGPASSWORD=$passwd

# Delete old data
psql -U postgres forgottenwars -t -c "SELECT 'drop table \"' || tablename || '\" cascade;' FROM pg_tables WHERE schemaname = 'public'" | psql -U postgres forgottenwars
