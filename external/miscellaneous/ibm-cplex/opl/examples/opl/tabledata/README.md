IBM OPL Database connection samples
-----------------------------------

The samples in this directory demonstrate how to use database connections implemented using the
OPL table data handler framework.

The database connections are provided as samples you can freely modify to adapt to your usage.

Database drivers are not provided as part of IBM CPLEX Optimization Studio. You will need to download
the driver you use. For JDBC, you will need to download a JDBC driver for your database. For ODBC you will need
to download an ODBC driver for your database. For SQLite, you need to download the SQLite client.

This directory contains:

* `10000x10int.db` - a SQLite example database.
* `tablehandler_csv.in` - The example data as CSV.
* `data.sql` - Generic SQL script to create the example database.
* `mssql_data.sql` - Transact-SQL script to create the example database with Microsoft SQL Server.
* `README.md` - This file.
* `sqlite-jdbc-3.45.2.0.jar` - JDBC driver for sqlite, for use with the SQL Lite example.
* `tablehandler.mod` - The model for the data table handler example.
* `tablehandler_csv.dat` - The `.dat` file to demonstrate reading data using CSVConnection.
* `tablehandler_jdbc.dat` - The `.dat` file to demonstrate reading data from a SQLite database using JDBCConnection.
* `tablehandler_odbc.dat` - The `.dat` file to demonstrate reading data from Microsoft SQL Server using ODBCConnection.
* `tablehandler_sqlite.dat` - The `.dat` file to demonstrate reading data from a SQLite database using SQLiteConnection.


CSVConnection
-------------

`tablehandler_csv.dat` demonstrate reading data from a `.csv` file using `CSVConnection`.
You run the example in a terminale console with:

```
$ oplrun tablehandler.mod tablehandler_csv.dat
```

JDBCConnection
--------------
`tablehandler_jdbc.dat` demonstrates reading data from a SQLite database using JDBCConnection. This
sample uses the `sqlite-jdbc-3.45.2.0.jar` JDBC driver.

If the JDBC driver for your database is in a JAR that is not on the default classpath, you need to tell the JVM used by OPL where it can find this code. This is done by passing the argument -classpath /path/to/driver.ssqlite-jdbc-3.45.2.0, either by setting environment variable ODMS_JAVA_ARGS or using command line argument -Xjavaargs for `oplrun` (or `oplrunjava` on non-Windows). Typically:
Then you need to download [slf4j-api-1.7.36.jar](https://search.maven.org/remotecontent?filepath=org/slf4j/slf4j-api/1.7.36/slf4j-api-1.7.36.jar)
```
$ set ODMS_JAVA_ARGS="-classpath path/to/sqlite-jdbc-3.45.2.0.jar:path/to/slf4j-api-1.7.36.jar"
```
Before you run the sample, you must point to `10000x10int.db` by setting the `ILO_JDBC_TEST` environment variable to its path.
For instance, on Windows, assuming you are in the `opl` directory:
```
$ set ILO_JDBC_TEST=%cd%/examples/opl/tabledata
```
On Unix depending on your `SHELL`:
```
$ export ILO_JDBC_TEST=$PWD/examples/opl/tabledata
```

You run the example in a terminal console with:
```
$ oplrun tablehandler.mod tablehandler_jdbc.dat
```

ODBCConnection
--------------
`tablehandler_odbc.dat` demonstrates reading data from a Microsoft SQL Server database using ODBCConnection.

You need to install ODBC drivers on your system, and edit the `.dat` with your ODBC connection string.

This example assumes you have a Microsoft SQL Server Express instance installed as SQLEXPRESS, and that the sample
database is named `table_handler` - If not, please check connection strings for the right instance and database names.

You can create example database in Microsoft SQL Server by running command (assuming your database is SQLEXPRESS on localhost):
```
C:\> sqlcmd -S .\SQLEXPRESS -i mssql_data.sql
```

You run the example in a terminal console with:
```
$ oplrun tablehandler.mod tablehandler_odbc.dat
```
