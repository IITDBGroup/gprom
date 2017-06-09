package org.gprom.jdbc.pawd;

public class QueryCostModel {

	public static long QueryCostModelMain(String Q) {
		return Q.hashCode();
	}

}
