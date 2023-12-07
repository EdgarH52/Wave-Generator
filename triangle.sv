module triangle(
  input clk,
  input [31:0] freq,
  input run, //new
  input [15:0] cycles, //new
  output reg [11:0] triangle,
  output reg ofs_kill
);

reg [47:0] phase; //new added 16 bits, these upper 16 bits should equal the number of elapsed cycles (lower 32 bits are 1 cycle)

reg [31:0] delta_phase;

reg [51:0] delta_phase_buffer;

reg [11:0] addr;

reg [15:0] elapsed_cycles; //new

assign delta_phase = delta_phase_buffer[31:0];

assign delta_phase_buffer = (freq[14:0] * 37'b1010011111000101101011000100011100011) >> 5'd20;

assign addr = phase[31:20];

assign elapsed_cycles = phase[47:32];
    
always@(posedge clk)
    begin
        if(run == 1'b0)  //new
            begin          //new
                phase <= 48'd0;   //new
            end           //new
        else if((elapsed_cycles < cycles) || (cycles == 16'd0))  //new
            begin
                if(addr < 11'd2047)
                    begin
                        triangle <= addr;
                        phase <= phase + delta_phase;
                        ofs_kill <= 1'b1;
                    end
                else
                    begin
                        triangle <= 12'd4095 - addr;
                        phase <= phase + delta_phase;
                        ofs_kill <= 1'b1;
                    end
            end
        else
            begin
                ofs_kill <= 1'b0;
            end
    end     
endmodule