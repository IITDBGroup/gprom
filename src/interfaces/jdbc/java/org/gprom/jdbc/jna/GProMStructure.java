package org.gprom.jdbc.jna;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

public abstract class GProMStructure extends Structure
{
	 public GProMStructure(Pointer address) { 
	    super(address); 
	    read(); 
	  } 
	 public GProMStructure() { 
	    super();
	 } 
}
