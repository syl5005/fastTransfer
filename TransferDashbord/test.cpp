/**************************************************************************
**   UpCloudEcosphere
**   Copyright (c) UpCloud C/C++ development team.
**   All rights 2016 reserved.
**   Author : Lynzabo
**************************************************************************/
/*int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QByteArray help =
            "qt-ftpsearcher -iplist [file] -ip [ip]\r\n"
            "  {-database [name] -user [...] -password [...] "
            "-host [...] -port [...]}\r\n"
            "    -cutip            cut active ip from file specified in -iplist\r\n"
            "    -ftp_user [user]  default is empty\r\n"
            "    -ftp_pass [pwd]   default is empty\r\n"
            "    -ftp_port [port]  default = 21\r\n"
            "    -ftp_code [name]  ftp encoding, it will be stored to table3\r\n"
            "                      (Apple Roman, Big5, Big5-HKSCS, EUC-JP, EUC-KR,\r\n"
            "                       GB18030-0, IBM 850, IBM 866, IBM 874, ISO 2022-JP,\r\n"
            "                       ISO 8859-1 to 10, ISO 8859-13 to 16, {Iscii-Bng,\r\n"
            "                       Dev, Gjr, Knd, Mlm, Ori, Pnj, Tlg, and Tml},\r\n"
            "                       JIS X 0201, JIS X 0208, KOI8-R, KOI8-U, MuleLao-1,\r\n"
            "                       ROMAN8, Shift-JIS, TIS-620, TSCII, UTF-8, UTF-16,\r\n"
            "                       UTF-16BE, UTF-16LE, UTF-32, UTF-32BE, UTF-32LE,\r\n"
            "                       Windows-1250 to 1258, WINSAMI2)\r\n"
            "                      default = Windows-1251\r\n"
            "    -table1 [name]    default = ftp_files\r\n"
            "    -table2 [name]    default = ftp_info\r\n"
            "    -table3 [name]    default = ftp_encoding\r\n"
            "    -path  [path]     specify path on ftp (default is '/')\r\n"
            "                      you can use this option multiple times:\r\n"
            "                          -path [path1] -path [path2] -path [path3]...\r\n"
            "    -log   [file]     log-file\r\n"
            "    -min   [size]     minimum file size that can be resolved,\r\n"
            "                      less file will be ignored, for example:\r\n"
            "                      '512' (bytes), '1k', '1m' or '1g'\r\n"
            "                      (default = 0)\r\n"
            "    -num   [num]      number of child porcesses\r\n"
            "    -time  [sec]      ip list to scan from table2 will be selected with \r\n"
            "                      condition that last update was N seconds ago or more\r\n"
            "    -begin [ip]       begin from ip\r\n"
            "    -scan             only scan iplist\r\n"
            "    -optimize         optimize tables 1,2,3 and exit\r\n"
            "                      (OPTIMIZE TABLE and ANALYZE TABLE)\r\n"
            "    -check            check and recode characters to utf-8\r\n"
            "                      (UTF-8, KOI8-R, Windows-1251, etc...)\r\n"
            "    -daemon           start as daemon\r\n"
            "    -h, -help         show this help\r\n"
            "    -v                version information\r\n"
            "For more information see manual page and help files in html.\r\n";

    QByteArray vers = "Version: "+QByteArray(VERSION)+
            "\r\n2008-2009 (c) Boris Pek (aka \\\\Tehnick)\r\n";

    if(argc==1){
        printf(help.constData());
        return 0;
    }

    bool optimize = false;
    QStringList args = app.arguments();
    QString arg;
    for(int i=0;i<args.size();++i){
        arg = args[i].toLower();
        if(arg=="-h"||arg=="-help"||arg=="--help"){
            printf(help.constData());
            return 0;
        }
        if(arg=="-v"){
            printf(vers.constData());
            return 0;
        }
        if(arg=="-daemon"){
            QString prog = args[0];
            args.removeAt(i);
            args.removeAt(0);
            QProcess *process = new QProcess();
            process->startDetached(prog,args);
            QByteArray time = QDateTime::currentDateTime()
                    .toString("hh:mm:ss").toAscii();
            printf("Process was detached at %s.\r\n",time.data());
            return 0;
        }
        if(arg=="-optimize"){
            optimize = true;
        }
    }

    Sorter sorter;
    if(!sorter.conf(args)) return 1;
    sorter.start();
    if(optimize) return 0;
    else return app.exec();
}
*/
