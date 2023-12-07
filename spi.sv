module spi(
    input [11:0] dac_a,
    input [11:0] dac_b,
    input clk,
    output reg cs,
    output reg ldac,
    output reg sdi,
    output reg two_Mhz_clock,
    output reg two_Mhz_sig
    );
    
    reg [4:0] div25;
    reg [5:0] div50;
    reg four_Mhz_sig;
    reg four_Mhz_sig_del;
    //reg two_Mhz_sig;
    reg [5:0] state;
    reg clock_enable;
    reg [4:0] data_counter;
    reg [4:0] data_counter_del;
    reg ab;
    reg clk_2MHz;
    reg clk_2MHz_del;
    reg clk_2MHz_rsg;
    reg two_Mhz_clock_del;
    reg two_Mhz_clock_fall;
    reg clock_enable_del;
    wire clock_enable_rsg;
    
    reg [3:0] cnt1  ;
    reg [3:0] cnt2  ;
    
    wire [11:0] dac_a_unsigned;
    wire [11:0] dac_b_unsigned;
    
    s2u s2u_0(
    .signedNum(dac_a),
    .unsignedNum(dac_a_unsigned));
    
    s2u s2u_1(
    .signedNum(dac_b),
    .unsignedNum(dac_b_unsigned));
    
    always@(posedge clk)
        begin
	        if(div25==5'd25) 
	            begin 
	                four_Mhz_sig<=1'b1;div25<=5'd0;
	            end
			else if(div25==5'd0) 
		        begin 
		            four_Mhz_sig<=1'b0;div25<=5'd1;
		        end
			else begin four_Mhz_sig<=1'd0;div25<=div25+1'd1;end
        end

    always@(posedge clk)
        begin 
            if(four_Mhz_sig)
                clk_2MHz = ~clk_2MHz;
        end 
    
    always@(posedge clk)
        begin 
            clk_2MHz_del = clk_2MHz;
            two_Mhz_clock_del = two_Mhz_clock;
            clock_enable_del = clock_enable;
            if(two_Mhz_clock_fall)
            begin
                data_counter_del = data_counter;
            end 
        end 
    
    assign clk_2MHz_rsg = clk_2MHz & ~clk_2MHz_del;
    assign two_Mhz_clock_fall = ~two_Mhz_clock & two_Mhz_clock_del;
    assign clock_enable_rsg = ~clock_enable_del & clock_enable;
    
    assign two_Mhz_clock = (clock_enable)? clk_2MHz : 1'b1;

    assign two_Mhz_sig = clk_2MHz_rsg;
        
    always@(posedge two_Mhz_sig)
        begin
            if(state==6'd40)
                begin
                    state=6'd0;
                end
            else
                begin
                    state=state + 1'd1;
                end
        end
               
    always@(posedge clk)
    begin
        if(clock_enable_rsg)//no use
                data_counter <= 0;
        else if(two_Mhz_clock_fall)
        begin
            if(data_counter == 15)
                data_counter <= 0   ;
            else
                data_counter=data_counter+1;
        end 
    end 
   
    always @(posedge clk) begin
		//case (16-data_counter)
		case (15-data_counter_del)
		    5'd0: sdi=0;
		    5'd1: sdi=0;
		    5'd2: 
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[2];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[2];
		              end		    
		    5'd3:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[3];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[3];
		              end
		    5'd4:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[4];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[4];
		              end
		    5'd5:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[5];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[5];
		              end
		    5'd6: 
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[6];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[6];
		              end
		    5'd7:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[7];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[7];
		              end
		    5'd8:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[8];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[8];
		              end
		    5'd9:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[9];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[9];
		              end		    
		    5'd10:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[10];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[10];
		              end		    
		    5'd11:
		          if(ab==0)
		              begin
		                  sdi=dac_a_unsigned[11];
		              end
		          else
		              begin
		                  sdi=dac_b_unsigned[11];
		              end
		    5'd12: sdi=1;
		    5'd13: sdi=1;
		    5'd14: sdi=0;
		    5'd15: 
		          if(ab==0)
		              begin
		                  sdi=1'd0;
		              end
		          else
		              begin
		                  sdi=1'd1;
		              end
	    endcase
	end
    
    always @(posedge clk) begin
		case (state)
		    6'd0: ldac=1;
		    6'd1: begin cs<=0; ab<=0; end
		    6'd2: clock_enable=1;
		    6'd18: clock_enable=0;
		    6'd19: cs=1;
		    6'd20: begin cs<=0; ab<=1; end
		    6'd21: clock_enable=1;
		    6'd37: clock_enable=0;
		    6'd38: cs=1;
		    6'd39: ldac=0;
	    endcase
	end
endmodule