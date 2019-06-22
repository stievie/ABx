# systemd

Files to run the servers as daemos with systemd.

Put them into `/lib/systemd/system` and mark them as executeable. Then run 
`systemctl enable <somefile>` to automatically start the service when the network
is running.
