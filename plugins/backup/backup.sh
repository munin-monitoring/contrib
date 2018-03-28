#!/bin/sh

# Configuration directives, edit before first use.
BACKUP_DIR=/path/to/your/backups
# How old backups should be considered as non-yound anymore in [days].
LIFETIME=2

# The situation is critical if there are no young files, the backup is down.
case $1 in
   config)
        cat <<'EOM'
graph_title Number of young files at backup directory
graph_vlabel number
graph_category backup
autobackup.label number
autobackup.critical 1:
EOM
        exit 0;;
esac

printf "autobackup.value "
find "$BACKUP_DIR" -mtime "-$LIFETIME" | wc  -l
