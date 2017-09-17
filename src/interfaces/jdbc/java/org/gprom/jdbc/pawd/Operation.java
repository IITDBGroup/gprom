package org.gprom.jdbc.pawd;

public class Operation {
    private String Code;
    private OpType Op;
    public Operation(String code, OpType op) {
        Code = code;
        Op = op;
    }

    /**
     * dummy constructor
     */
    public Operation(){
        Code = "";
        Op = null;
    }
    /**
     * @return the code
     */
    public String getCode() {
        return Code;
    }
    /**
     * @param code the code to set
     */
    public void setCode(String code) {
        Code = code;
    }
    /**
     * @return the op
     */
    public OpType getOp() {
        return Op;
    }
    /**
     * @param op the op to set
     */
    public void setOp(OpType op) {
        Op = op;
    }
    /* (non-Javadoc)
     * @see java.lang.Object#toString()
     */
    @Override
    public String toString() {
        return "[Code=" + Code + ", Op=" + Op + "]";
    }

    public enum OpType{
        Query,
        Update
    }
    public enum Materialization{
        isMaterialized,
        notMaterialized
    }
    @Override
    public boolean equals(Object other){
        if (other == null) return false;
        if (other == this) return true;
        if (!(other instanceof Operation))return false;
        Operation otherOp = (Operation)other;
        return otherOp.getCode().equals(Code) && otherOp.Op.equals(Op);
    }

    @Override
    public int hashCode() {
        int result = Code.hashCode();
        result = 31 * result + Op.hashCode();
        return result;
    }
}
