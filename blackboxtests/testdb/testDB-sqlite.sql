/******************************************************************************
*******************************************************************************
*******************************************************************************
******* 	Table Defs     ************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

/******************************************************************************
******* 	simplistic tables					   ****************************
******************************************************************************/

-- TODO check whether they exit
DROP TABLE IF EXISTS R;
DROP TABLE IF EXISTS S;
DROP TABLE IF EXISTS T;
DROP TABLE IF EXISTS PK_R;
DROP TABLE IF EXISTS PK_S;
DROP TABLE IF EXISTS TEMP_TEST;
DROP TABLE IF EXISTS TEMP_DATE;
DROP TABLE IF EXISTS TEMP_EMP;
DROP TABLE IF EXISTS FBA_TEST;

CREATE TABLE R (
	A int,
	B int
);

INSERT INTO R VALUES (1,1);
INSERT INTO R VALUES (1,2);
INSERT INTO R VALUES (2,1);
INSERT INTO R VALUES (2,3);


CREATE TABLE S (
	C int,
	D int
);

INSERT INTO S VALUES (2,2);
INSERT INTO S VALUES (2,3);
INSERT INTO S VALUES (3,2);
INSERT INTO S VALUES (1,4);


CREATE TABLE T (
	E int,
	F int
);

INSERT INTO T VALUES (1,1);
INSERT INTO T VALUES (2,2);


CREATE TABLE PK_R (
	A int PRIMARY KEY,
	B int
);


INSERT INTO PK_R VALUES (1,1);
INSERT INTO PK_R VALUES (2,1);
INSERT INTO PK_R VALUES (3,3);


CREATE TABLE PK_S (
	C int,
	D int,
	PRIMARY KEY (C,D)
);

INSERT INTO PK_S VALUES (1,1);
INSERT INTO PK_S VALUES (1,2);
INSERT INTO PK_S VALUES (2,1);
INSERT INTO PK_S VALUES (2,2);


CREATE TABLE TEMP_TEST (
	A INT,
	B INT,
	T_BEGIN INT,
	T_END INT
);


INSERT INTO TEMP_TEST VALUES (1,1, 1,5);
INSERT INTO TEMP_TEST VALUES (1,1, 2,6);
INSERT INTO TEMP_TEST VALUES (1,1, 5,10);
INSERT INTO TEMP_TEST VALUES (1,2, 1,13);
INSERT INTO TEMP_TEST VALUES (1,2, 1,2);
INSERT INTO TEMP_TEST VALUES (2,1, 1,2);
INSERT INTO TEMP_TEST VALUES (2,1, 2,3);
INSERT INTO TEMP_TEST VALUES (2,1, 3,4);


CREATE TABLE TEMP_DATE (
	A INT,
	B INT,
	T_BEGIN DATE,
	T_END DATE
);

INSERT INTO TEMP_DATE VALUES (1,1, DATE('2017-01-01'), DATE('2017-02-01'));
INSERT INTO TEMP_DATE VALUES (1,1, DATE('2017-01-05'), DATE('2017-02-15'));
INSERT INTO TEMP_DATE VALUES (1,1, DATE('2017-02-01'), DATE('2017-04-07'));


CREATE TABLE TEMP_EMP (
  EMPID INT,
  SALARY INT,
  FROM_DATE DATE,
  DATE DATE
);

INSERT INTO TEMP_EMP VALUES (10034, 53112, DATE('1998-04-11'), DATE('1999-04-11'));
INSERT INTO TEMP_EMP VALUES (10034, 53164, DATE('1999-04-11'), DATE('1999-10-31'));
INSERT INTO TEMP_EMP VALUES (10035, 41538, DATE('1988-09-05'), DATE('1989-09-05'));
INSERT INTO TEMP_EMP VALUES (10035, 45131, DATE('1989-09-05'), DATE('1990-09-05'));
INSERT INTO TEMP_EMP VALUES (10035, 45629, DATE('1990-09-05'), DATE('1991-09-05'));
INSERT INTO TEMP_EMP VALUES (10035, 48360, DATE('1991-09-05'), DATE('1992-09-04'));
INSERT INTO TEMP_EMP VALUES (10035, 50664, DATE('1992-09-04'), DATE('1993-09-04'));
INSERT INTO TEMP_EMP VALUES (10035, 53060, DATE('1993-09-04'), DATE('1994-09-04'));
INSERT INTO TEMP_EMP VALUES (10035, 56640, DATE('1994-09-04'), DATE('1995-09-04'));

CREATE TABLE FBA_TEST (
	A INT,
	B INT
);
