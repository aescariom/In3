package org.escario.in3.model;

/**
 * Created by alejandro on 7/6/15.
 */
public class TransmittedObject {

    public byte type;
    public Float value;

    public TransmittedObject(byte type, Float value) {
        this.type = type;
        this.value = value;
    }
}
