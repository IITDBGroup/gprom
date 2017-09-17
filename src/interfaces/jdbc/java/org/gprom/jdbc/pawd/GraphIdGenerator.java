package org.gprom.jdbc.pawd;

public class GraphIdGenerator {
    private static final GraphIdGenerator ourInstance = new GraphIdGenerator();
    private long idCounter = 0;

    public static GraphIdGenerator getInstance() {
        return ourInstance;
    }

    private GraphIdGenerator() {
    }

    public long getIdCounter() {
        return idCounter;
    }
    public long generateNextId(){
        return idCounter++;
    }

    public void setIdCounter(long idCounter) {
        this.idCounter = idCounter;
    }
}
