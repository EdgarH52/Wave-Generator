module phase_accumulator(
  input clk,
  input [31:0] freq_a,
  input [31:0] freq_b,
  output [11:0] addr_a,
  output [11:0] addr_b
);

reg [31:0] delta_phase_a;
reg [31:0] delta_phase_b;

//reg [51:0] delta_phase_a_buffer;
//reg [31:0] delta_phase_b_buffer;

assign delta_phase_a = (freq_a[14:0] * 37'b1010011111000101101011000100011100011) >> 5'd20; //85899.34592 
assign delta_phase_b = (freq_b[14:0] * 37'b1010011111000101101011000100011100011) >> 5'd20; //85899.34592  1/50k * 2^32

//assign delta_phase_a = delta_phase_a_buffer[51:20];
//assign delta_phase_b = delta_phase_b_buffer[51:20];

reg [31:0] phase_a;
reg [31:0] phase_b;

assign addr_a = phase_a[31:20];
assign addr_b = phase_b[31:20];

reg [10:0] counter;
reg kHz50;

always@(posedge clk)
    begin
	    if(counter==11'd2000)
	        begin 
	            kHz50<=1'b1;counter<=11'd0;
	        end
	    else if(kHz50==11'd0)
		    begin 
		        kHz50<=1'b0;counter<=11'd1;
		    end
		else begin kHz50<=1'd0;counter<=counter+1'd1;end
    end
    
always@(posedge kHz50)
    begin
	    phase_a <= phase_a + delta_phase_a;
	    phase_b <= phase_b + delta_phase_b;
    end

endmodule