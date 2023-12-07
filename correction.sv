module correction#(
  parameter [15:0] gainA = 16'b0001001111001011,//.96
  parameter [15:0] gainB = 16'b0001001111001011,
  parameter [9:0] ofsA = 10'b0010000001,
  parameter [9:0] ofsB = 10'b0010010010
)(
  input signed [15:0] dac_a,
  input signed [15:0] dac_b,
  output reg [11:0] a_corr,
  output reg [11:0] b_corr
);

  reg signed [31:0] dac_a_ext;
  reg signed [31:0] dac_b_ext;

  reg signed [31:0] resultA;
  reg signed [31:0] resultB;

  assign dac_a_ext = dac_a;
  assign dac_b_ext = dac_b;


  assign  resultA = ((dac_a_ext + ofsA) * gainA);
  assign  resultB = ((dac_b_ext - ofsB) * gainB);



   assign  a_corr = resultA[27:16];
   assign  b_corr = resultB[27:16];


endmodule
