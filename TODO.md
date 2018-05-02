Skills, Effects, Professions, etc. to DB
Additional to UUID add an Index which is used to encode it.

* abclient, abserv: LoginProtocol should use Account UUID instead of name except for first login.
* abdata: Flush cache: When it failes do not try endlessly
* abclient, abserv: Delete Mail /delete <index> | all | read
* Check https://github.com/im95able/Rea for FL?
