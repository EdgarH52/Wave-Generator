module s2u(
    input [11:0] signedNum,
    output reg [11:0] unsignedNum
    );
    
    assign unsignedNum[11] = signedNum[11];
    assign unsignedNum[10:0] = 11'd2047 - signedNum[10:0];
endmodule