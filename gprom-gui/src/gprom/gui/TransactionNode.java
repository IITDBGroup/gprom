package gprom.gui;

import java.sql.Timestamp;


public class TransactionNode {
	private String query;
	private int SCN;
	private Timestamp timeStamp;
	
	
	public TransactionNode(String query, int sCN, Timestamp timeStamp) {
		super();
		this.query = query;
		SCN = sCN;
		this.timeStamp = timeStamp;
	}
	public String getQuery() {
		return query;
	}
	public void setQuery(String query) {
		this.query = query;
	}
	public int getSCN() {
		return SCN;
	}
	public void setSCN(int sCN) {
		SCN = sCN;
	}
	public Timestamp getTimeStamp() {
		return timeStamp;
	}
	public void setTimeStamp(Timestamp timeStamp) {
		this.timeStamp = timeStamp;
	}
	
	

}
