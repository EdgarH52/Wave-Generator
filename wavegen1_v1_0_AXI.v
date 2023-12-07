// AXI4-lite GPIO IP implementation
// (gpio_v1_0_AXI.v)
// Jason Losh based on Xilinx IP tool auto-generated file
//
// Contains:
// AXI4-lite interface
// GPIO memory-mapped interface
// GPIO port interface and implemention
// GPIO interrupt generation

`timescale 1 ns / 1 ps


    module wavegen1_v1_0_AXI #
    (
        // Bit width of S_AXI address bus
        parameter integer C_S_AXI_ADDR_WIDTH = 5
    )
    (
        // Ports to top level module (what makes this the GPIO IP module)
        input wire spi_clk_in,
        output wire sdi,
        output wire ldac,
        output wire cs,
        output wire spi_clk_out,
        output wire overload,
        output wire [11:0] addr_a,
        input wire [15:0] sine_a,
        output wire [11:0] addr_b,
        input wire [15:0] sine_b,
        output wire [11:0] sine_test,

        // AXI clock and reset        
        input wire S_AXI_ACLK,
        input wire S_AXI_ARESETN,

        // AXI write channel
        // address:  add, protection, valid, ready
        // data:     data, byte enable strobes, valid, ready
        // response: response, valid, ready 
        input wire [C_S_AXI_ADDR_WIDTH-1:0] S_AXI_AWADDR,
        input wire [2:0] S_AXI_AWPROT,
        input wire S_AXI_AWVALID,
        output wire S_AXI_AWREADY,
        
        input wire [31:0] S_AXI_WDATA,
        input wire [3:0] S_AXI_WSTRB,
        input wire S_AXI_WVALID,
        output wire  S_AXI_WREADY,
        
        output wire [1:0] S_AXI_BRESP,
        output wire S_AXI_BVALID,
        input wire S_AXI_BREADY,
        
        // AXI read channel
        // address: add, protection, valid, ready
        // data:    data, resp, valid, ready
        input wire [C_S_AXI_ADDR_WIDTH-1:0] S_AXI_ARADDR,
        input wire [2:0] S_AXI_ARPROT,
        input wire S_AXI_ARVALID,
        output wire S_AXI_ARREADY,
        
        output wire [31:0] S_AXI_RDATA,
        output wire [1:0] S_AXI_RRESP,
        output wire S_AXI_RVALID,
        input wire S_AXI_RREADY
    );

    // Internal registers
    reg [31:0] mode;
    reg [31:0] run;
    reg [31:0] freq_a;
    reg [31:0] freq_b;
    reg [31:0] offset;
    reg [31:0] amplitude;
    reg [31:0] dtycyc;
    reg [31:0] cycles;
    
    // Register map
    // ofs  fn
    //   0  data (r/w)
    //   4  out (r/w)
    //   8  od (r/w)
    //  12  int_enable (r/w)
    //  16  int_positive (r/w)
    //  20  int_negative (r/w)
    //  24  int_edge_mode (r/w)
    //  28  int_status_clear (r/w1c)
    
    // Register numbers
    localparam integer MODE_REG             = 3'b000;
    localparam integer RUN_REG              = 3'b001;
    localparam integer FREQ_A_REG           = 3'b010;
    localparam integer FREQ_B_REG           = 3'b011;
    localparam integer OFFSET_REG           = 3'b100;
    localparam integer AMPLITUDE_REG        = 3'b101;
    localparam integer DTYCYC_REG           = 3'b110;
    localparam integer CYCLES_REG           = 3'b111;
    
    // AXI4-lite signals
    reg axi_awready;
    reg axi_wready;
    reg [1:0] axi_bresp;
    reg axi_bvalid;
    reg axi_arready;
    reg [31:0] axi_rdata;
    reg [1:0] axi_rresp;
    reg axi_rvalid;
    
    // friendly clock, reset, and bus signals from master
    wire axi_clk           = S_AXI_ACLK;
    wire axi_resetn        = S_AXI_ARESETN;
    wire [31:0] axi_awaddr = S_AXI_AWADDR;
    wire axi_awvalid       = S_AXI_AWVALID;
    wire axi_wvalid        = S_AXI_WVALID;
    wire [3:0] axi_wstrb   = S_AXI_WSTRB;
    wire axi_bready        = S_AXI_BREADY;
    wire [31:0] axi_araddr = S_AXI_ARADDR;
    wire axi_arvalid       = S_AXI_ARVALID;
    wire axi_rready        = S_AXI_RREADY;    
    
    // assign bus signals to master to internal reg names
    assign S_AXI_AWREADY = axi_awready;
    assign S_AXI_WREADY  = axi_wready;
    assign S_AXI_BRESP   = axi_bresp;
    assign S_AXI_BVALID  = axi_bvalid;
    assign S_AXI_ARREADY = axi_arready;
    assign S_AXI_RDATA   = axi_rdata;
    assign S_AXI_RRESP   = axi_rresp;
    assign S_AXI_RVALID  = axi_rvalid;
    
    // Handle gpio input metastability safely
//    reg [31:0] read_port_data;
//    reg [31:0] pre_read_port_data;
//    always_ff @ (posedge(axi_clk))
//    begin
//        pre_read_port_data <= gpio_data_in;
//        read_port_data <= pre_read_port_data;
//    end

    // Assert address ready handshake (axi_awready) 
    // - after address is valid (axi_awvalid)
    // - after data is valid (axi_wvalid)
    // - while configured to receive a write (aw_en)
    // De-assert ready (axi_awready)
    // - after write response channel ready handshake received (axi_bready)
    // - after this module sends write response channel valid (axi_bvalid) 
    wire wr_add_data_valid = axi_awvalid && axi_wvalid;
    reg aw_en;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_awready <= 1'b0;
            aw_en <= 1'b1;
        end
        else
        begin
            if (wr_add_data_valid && ~axi_awready && aw_en)
            begin
                axi_awready <= 1'b1;
                aw_en <= 1'b0;
            end
            else if (axi_bready && axi_bvalid)
                begin
                    aw_en <= 1'b1;
                    axi_awready <= 1'b0;
                end
            else           
                axi_awready <= 1'b0;
        end 
    end

    // Capture the write address (axi_awaddr) in the first clock (~axi_awready)
    // - after write address is valid (axi_awvalid)
    // - after write data is valid (axi_wvalid)
    // - while configured to receive a write (aw_en)
    reg [C_S_AXI_ADDR_WIDTH-1:0] waddr;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
            waddr <= 0;
        else if (wr_add_data_valid && ~axi_awready && aw_en)
            waddr <= axi_awaddr;
    end

    // Output write data ready handshake (axi_wready) generation for one clock
    // - after address is valid (axi_awvalid)
    // - after data is valid (axi_wvalid)
    // - while configured to receive a write (aw_en)
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
            axi_wready <= 1'b0;
        else
            axi_wready <= (wr_add_data_valid && ~axi_wready && aw_en);
    end       

    // Write data to internal registers
    // - after address is valid (axi_awvalid)
    // - after write data is valid (axi_wvalid)
    // - after this module asserts ready for address handshake (axi_awready)
    // - after this module asserts ready for data handshake (axi_wready)
    // write correct bytes in 32-bit word based on byte enables (axi_wstrb)
    // int_clear_request write is only active for one clock
    wire wr = wr_add_data_valid && axi_awready && axi_wready;
    integer byte_index;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            mode[31:0] <= 32'b0;
            run <= 32'b0;
            freq_a <= 32'b0;
            freq_b <= 32'b0;
            offset <= 32'b0;
            amplitude <= 32'b0;
            dtycyc <= 32'b0;
            cycles <= 32'b0;
        end 
        else 
        begin
            if (wr)
            begin
                case (axi_awaddr[4:2])
                    MODE_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if ( axi_wstrb[byte_index] == 1) 
                                mode[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    RUN_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                run[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    FREQ_A_REG: 
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                freq_a[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    FREQ_B_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                freq_b[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    OFFSET_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1) 
                                offset[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    AMPLITUDE_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                amplitude[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    DTYCYC_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                dtycyc[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    CYCLES_REG:
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                cycles[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                endcase
            end
        end
    end    

    // Send write response (axi_bvalid, axi_bresp)
    // - after address is valid (axi_awvalid)
    // - after write data is valid (axi_wvalid)
    // - after this module asserts ready for address handshake (axi_awready)
    // - after this module asserts ready for data handshake (axi_wready)
    // Clear write response valid (axi_bvalid) after one clock
    wire wr_add_data_ready = axi_awready && axi_wready;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_bvalid  <= 0;
            axi_bresp   <= 2'b0;
        end 
        else
        begin    
            if (wr_add_data_valid && wr_add_data_ready && ~axi_bvalid)
            begin
                axi_bvalid <= 1'b1;
                axi_bresp  <= 2'b0;
            end
            else if (S_AXI_BREADY && axi_bvalid) 
                axi_bvalid <= 1'b0; 
        end
    end   

    // In the first clock (~axi_arready) that the read address is valid
    // - capture the address (axi_araddr)
    // - output ready (axi_arready) for one clock
    reg [C_S_AXI_ADDR_WIDTH-1:0] raddr;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_arready <= 1'b0;
            raddr <= 32'b0;
        end 
        else
        begin    
            // if valid, pulse ready (axi_rready) for one clock and save address
            if (axi_arvalid && ~axi_arready)
            begin
                axi_arready <= 1'b1;
                raddr  <= axi_araddr;
            end
            else
                axi_arready <= 1'b0;
        end 
    end       
        
    // Update register read data
    // - after this module receives a valid address (axi_arvalid)
    // - after this module asserts ready for address handshake (axi_arready)
    // - before the module asserts the data is valid (~axi_rvalid)
    //   (don't change the data while asserting read data is valid)
    wire rd = axi_arvalid && axi_arready && ~axi_rvalid;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_rdata <= 32'b0;
        end 
        else
        begin    
            if (rd)
            begin
		// Address decoding for reading registers
		case (raddr[4:2])
		    MODE_REG: 
		        axi_rdata <= mode;
		    RUN_REG:
		        axi_rdata <= run;
		    FREQ_A_REG: 
		        axi_rdata <= freq_a;
		    FREQ_B_REG: 
			    axi_rdata <= freq_b;
		    OFFSET_REG:
			    axi_rdata <= offset;
		    AMPLITUDE_REG:
			    axi_rdata <= amplitude;
		    DTYCYC_REG:
			    axi_rdata <= dtycyc;
		    CYCLES_REG:
		        axi_rdata <= cycles;
		endcase
            end   
        end
    end    

    // Assert data is valid for reading (axi_rvalid)
    // - after address is valid (axi_arvalid)
    // - after this module asserts ready for address handshake (axi_arready)
    // De-assert data valid (axi_rvalid) 
    // - after master ready handshake is received (axi_rready)
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
            axi_rvalid <= 1'b0;
        else
        begin
            if (axi_arvalid && axi_arready && ~axi_rvalid)
            begin
                axi_rvalid <= 1'b1;
                axi_rresp <= 2'b0;
            end   
            else if (axi_rvalid && axi_rready)
                axi_rvalid <= 1'b0;
        end
    end    

    // pin control
    // OUT LATCH ODR   PIN
    //  0    x    x    hi-Z
    //  1    0    x     0
    //  1    1    0     1
    //  1    1    1    hi-Z
//    genvar j;
//    for (j = 0; j < 32; j = j + 1)
//    begin
//        assign gpio_data_oe[j] = out[j] && (!latch_data[j] || !od[j]);
//    end
//    assign gpio_data_out = latch_data;
    
    // Interrupt generation
//    integer i;
//    reg [31:0] last_read_port_data;
//    always_ff @ (posedge axi_clk)
//    begin
//        if (axi_resetn == 1'b0)
//        begin
//            last_read_port_data <= 32'b0;
//            int_status <= 32'b0;
//        end
//        else if (int_clear_request != 32'b0)
//            int_status <= int_status & ~int_clear_request;
//        else
//        begin
//            last_read_port_data <= read_port_data;
//            for (i = 0; i < 32; i = i + 1)
//            begin
//                if (int_enable[i])
//                begin
//                    if (int_edge_mode[i])
//                    begin
//                        if (int_positive[i] && read_port_data[i] && !last_read_port_data[i])
//                            int_status[i] <= 1'b1;
//                        if (int_negative[i] && !read_port_data[i] && last_read_port_data[i])
//                            int_status[i] <= 1'b1;
//                    end
//                    else
//                    begin
//                        if (int_positive[i] && read_port_data[i])
//                            int_status[i] <= 1'b1;
//                        if (int_negative[i] && !read_port_data[i])
//                            int_status[i] <= 1'b1;
//                    end
//                end
//            end
//        end
//    end
//    assign intr = int_status != 32'b0;

    reg [15:0] dac_a;
    reg [15:0] dac_b;
    
    reg [31:0] dac_a_buff;
    reg [31:0] dac_b_buff;
    
    reg [15:0] abs_a;
    reg [15:0] abs_b;
    
    reg [10:0] count;
    reg kHz50;
    
    reg spi_latch; //new
    reg [15:0] dac_a_latch; //new
    reg [15:0] dac_b_latch; //new
    assign spi_latch = ldac; //new
    
    wire square_a;
    wire [11:0] triangle_a;
    wire [11:0] sawtooth_a;
    wire square_a_ofs_kill;
    wire triangle_a_ofs_kill;
    wire sawtooth_a_ofs_kill;
    wire sine_a_ofs_kill;
    
    wire square_b;
    wire [11:0] triangle_b;
    wire [11:0] sawtooth_b;
    wire square_b_ofs_kill;
    wire triangle_b_ofs_kill;
    wire sawtooth_b_ofs_kill;
    wire sine_b_ofs_kill;

    localparam integer DC_MODE            = 3'b000;
    localparam integer SINE_MODE          = 3'b001;
    localparam integer SAWTOOTH_MODE      = 3'b010;
    localparam integer TRIANGLE_MODE      = 3'b011;
    localparam integer SQUARE_MODE        = 3'b100;
    localparam integer ARB_MODE           = 3'b101;
    
    always@(negedge spi_clk_in)
	begin
		if(count==11'd1998) begin kHz50<=1'b1;count<=11'd1999;end
			else
		if(count==11'd1999) begin kHz50<=1'b0;count<=0;end
			else begin kHz50<=1'b0;count<=count+1'b1;end
	end

    always_ff @(posedge spi_clk_in) 
        begin
            case(mode[2:0])
                DC_MODE: 
                        if(run[0] == 1'b1)
                            dac_a <= offset[15:0];
                        else
                            dac_a <= 16'd0;
                SINE_MODE: 
                        if(run[0] == 1'b1)
                            begin
                                if(sine_a[15] == 1'b1)
                                    begin
                                        abs_a <= ~(sine_a - 1);              
                                        dac_a_buff <= (abs_a * amplitude[15:0]) >> 4'd12;
                                        dac_a <= ~(dac_a_buff[15:0]) + 1 + (offset[15:0] * sine_a_ofs_kill);
                                    end
                                else
                                    begin
                                        dac_a_buff <= (sine_a * amplitude[15:0]) >> 4'd12;
                                        dac_a <= dac_a_buff[15:0] + (offset[15:0] * sine_a_ofs_kill);
                                    end
                            end
                        else
                            dac_a <= 16'd0;
                SAWTOOTH_MODE: 
                        if(run[0] == 1'b1)
                            begin
                                dac_a_buff <= ((sawtooth_a * amplitude[15:0]) >> 4'd12) + (offset[15:0] * sawtooth_a_ofs_kill);
                                dac_a <= dac_a_buff[15:0];                            
                            end
                        else
                            dac_a <= 16'd0;
                TRIANGLE_MODE: 
                        if(run[0] == 1'b1)
                            begin
                                dac_a_buff <= ((triangle_a * amplitude[15:0]) >> 4'd11) + (offset[15:0] * triangle_a_ofs_kill);
                                dac_a <= dac_a_buff[15:0];                            
                            end
                        else
                            dac_a <= 16'd0;
                SQUARE_MODE: 
                        if(run[0] == 1'b1)
                            begin
                                dac_a <= (square_a * amplitude[15:0]) + (offset[15:0] * square_a_ofs_kill);
                            end
                        else
                            dac_a <= 16'd0;
                ARB_MODE: 
                        dac_a <= 16'd0;
             endcase
        end

    always_ff @(posedge spi_clk_in) 
        begin
            case(mode[5:3])
                DC_MODE: 
                        if(run[1] == 1'b1)
                            dac_b <= offset[31:16];
                        else
                            dac_b <= 16'd0;
                SINE_MODE: 
                        if(run[1] == 1'b1)
                            begin
                                if(sine_b[15] == 1'b1)
                                    begin
                                        abs_b <= ~(sine_b - 1);              
                                        dac_b_buff <= (abs_b * amplitude[31:16]) >> 4'd12;
                                        dac_b <= ~(dac_b_buff[15:0]) + 1 + (offset[31:16] * sine_b_ofs_kill);
                                    end
                                else
                                    begin
                                        dac_b_buff <= (sine_b * amplitude[31:16]) >> 4'd12;
                                        dac_b <= dac_b_buff[15:0] + (offset[31:16] * sine_b_ofs_kill);
                                    end
                            end
                        else
                            dac_b <= 16'd0;
                SAWTOOTH_MODE: 
                        if(run[1] == 1'b1)
                            begin
                                dac_b_buff <= ((sawtooth_b * amplitude[31:16]) >> 4'd12) + (offset[31:16] * sawtooth_b_ofs_kill);
                                dac_b <= dac_b_buff[15:0];                            
                            end
                        else
                            dac_b <= 16'd0;
                TRIANGLE_MODE: 
                        if(run[1] == 1'b1)
                            begin
                                dac_b_buff <= ((triangle_b * amplitude[31:16]) >> 4'd11) + (offset[31:16] * triangle_b_ofs_kill);
                                dac_b <= dac_b_buff[15:0];                            
                            end
                        else
                            dac_b <= 16'd0;
                SQUARE_MODE: 
                        if(run[1] == 1'b1)
                            begin
                                dac_b <= (square_b * amplitude[31:16]) + (offset[31:16] * square_b_ofs_kill);
                            end
                        else
                            dac_b <= 16'd0;
                ARB_MODE: 
                        dac_b <= 16'd0;
             endcase
        end
    
    square square_0(
      .clk(kHz50),
      .freq(freq_a),
      .duty(dtycyc[15:0]),
      .onoff(square_a),
      .run(run[0]),
      .cycles(cycles[15:0]),
      .ofs_kill(square_a_ofs_kill)
    );
    
    triangle triangle_0(
      .clk(kHz50),
      .freq(freq_a),
      .triangle(triangle_a),
      .run(run[0]),
      .cycles(cycles[15:0]),
      .ofs_kill(triangle_a_ofs_kill)
    );
    
    sawtooth sawtooth_0(
      .clk(kHz50),
      .freq(freq_a),
      .sawtooth(sawtooth_a),
      .run(run[0]),
      .cycles(cycles[15:0]),
      .ofs_kill(sawtooth_a_ofs_kill)
    );
    
    square square_1(
      .clk(kHz50),
      .freq(freq_b),
      .duty(dtycyc[31:16]),
      .onoff(square_b),
      .run(run[1]),
      .cycles(cycles[31:16]),
      .ofs_kill(square_b_ofs_kill)
    );
    
    triangle triangle_1(
      .clk(kHz50),
      .freq(freq_b),
      .triangle(triangle_b),
      .run(run[1]),
      .cycles(cycles[31:16]),
      .ofs_kill(triangle_b_ofs_kill)
    );
    
    sawtooth sawtooth_1(
      .clk(kHz50),
      .freq(freq_b),
      .sawtooth(sawtooth_b),
      .run(run[1]),
      .cycles(cycles[31:16]),
      .ofs_kill(sawtooth_b_ofs_kill)
    );

    phase_accumulator phase_0(
      .kHz50(kHz50),
      .freq_a(freq_a),
      .freq_b(freq_b),
      .addr_a(addr_a),
      .addr_b(addr_b),
      .run_a(run[0]),
      .run_b(run[1]),
      .cycles_a(cycles[15:0]),
      .cycles_b(cycles[31:16]),
      .ofs_kill_a(sine_a_ofs_kill),
      .ofs_kill_b(sine_b_ofs_kill)
    );
    
    wire [11:0] a_corr;
    wire [11:0] b_corr;
    
    always_ff@(negedge spi_latch)
        begin
            dac_a_latch <= dac_a;
            dac_b_latch <= dac_b;
        end
    
    correction corr_0(
    //.dac_a(dac_a),
    //.dac_b(dac_b),
    .dac_a(dac_a_latch),
    .dac_b(dac_b_latch),
    .a_corr(a_corr),
    .b_corr(b_corr));
    
    spi spi_0(
    .clk(spi_clk_in),
    .dac_a(a_corr),
    .dac_b(b_corr),
    .cs(cs),
    .ldac(ldac),
    .sdi(sdi),
    .two_Mhz_clock(spi_clk_out));
    
endmodule
