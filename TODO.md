Skills, Effects, Professions, etc. to DB
Additional to UUID add an Index which is used to encode it.

* abdata: Create should not be blocking. New method Create/CreateSynch? Or synch flag for all methods?
* abdata: Sudden disconnects to MySQL CHECK
* abclient, abserv: LoginProtocol should use Account UUID instead of name except for first login.
* abclient, abserv: LoginProtocol, GameProtocol should use Game UUID instead of name.
* Check https://github.com/im95able/Rea for FL?
