package org.gprom.jdbc.pawd;

public interface QueryCostModel {

	public static long QueryCostModelMain(String Q) {
		return Q.hashCode();
	}

}
