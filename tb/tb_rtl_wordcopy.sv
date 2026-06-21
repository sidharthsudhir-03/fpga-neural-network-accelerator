`timescale 1ps / 1ps

module tb_rtl_wordcopy;
    // Clock and reset
    reg clk;
    reg rst_n;

    // Slave interface signals (CPU-facing)
    wire slave_waitrequest;
    reg [3:0] slave_address;
    reg slave_read;
    wire [31:0] slave_readdata;
    reg slave_write;
    reg [31:0] slave_writedata;

    // Master interface signals (SDRAM-facing)
    reg master_waitrequest;
    wire [31:0] master_address;
    wire master_read;
    reg [31:0] master_readdata;
    reg master_readdatavalid;
    wire master_write;
    wire [31:0] master_writedata;

    // Instantiate the wordcopy module
    wordcopy uut (
        .clk(clk),
        .rst_n(rst_n),
        // Slave interface
        .slave_waitrequest(slave_waitrequest),
        .slave_address(slave_address),
        .slave_read(slave_read),
        .slave_readdata(slave_readdata),
        .slave_write(slave_write),
        .slave_writedata(slave_writedata),
        // Master interface
        .master_waitrequest(master_waitrequest),
        .master_address(master_address),
        .master_read(master_read),
        .master_readdata(master_readdata),
        .master_readdatavalid(master_readdatavalid),
        .master_write(master_write),
        .master_writedata(master_writedata)
    );

    // Clock generation
    initial clk = 0;
    always #5 clk = ~clk; // 100 MHz clock

    // Reset generation
    initial begin
        rst_n = 0;
        #20; // Hold reset for 20 ns
        rst_n = 1;
    end

    // Simple SDRAM memory model
    reg [31:0] memory [0:1023]; // 1024 words of memory

    // Emulate SDRAM controller behavior
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            master_waitrequest <= 0;
            master_readdata <= 32'd0;
            master_readdatavalid <= 0;
        end else begin
            // Default values
            master_waitrequest <= 0; // Always ready to accept requests

            // Handle read requests
            if (master_read && !master_waitrequest) begin
                // Read data from memory after a delay
                master_readdata <= memory[master_address]; // Assuming word-aligned addresses
                master_readdatavalid <= 1;
            end else begin
                master_readdatavalid <= 0;
            end

            // Handle write requests
            if (master_write && !master_waitrequest) begin
                // Write data to memory
                memory[master_address] <= master_writedata;
            end
        end
    end

    // Task to write to slave interface
    task cpu_write(input [3:0] address, input [31:0] data);
        begin
            @(posedge clk);
            slave_write = 1;
            slave_read = 0;
            slave_address = address;
            slave_writedata = data;
            // Wait for waitrequest to be deasserted
            while (slave_waitrequest) begin
                @(posedge clk);
            end
            @(posedge clk);
            slave_write = 0;
            slave_writedata = 32'd0;
        end
    endtask

    // Task to read from slave interface
    task cpu_read(input [3:0] address);
        begin
            @(posedge clk);
            slave_read = 1;
            slave_write = 0;
            slave_address = address;
            // Wait for waitrequest to be deasserted
            while (slave_waitrequest) begin
                @(posedge clk);
            end
            @(posedge clk);
            slave_read = 0;
        end
    endtask

    // Initialize memory and perform the copy
    initial begin
        // Wait for reset to complete
        @(posedge rst_n);

        // Initialize source data in memory
        memory[100] = 32'hDEADBEEF;
        memory[104] = 32'hCAFEBABE;
        memory[108] = 32'h12345678;
        memory[112] = 32'h87654321;

        // Clear slave interface signals
        slave_write = 0;
        slave_read = 0;
        slave_address = 0;
        slave_writedata = 0;

        // Wait a few clock cycles
        repeat (5) @(posedge clk);

        // Set up copy parameters
        cpu_write(4'd2, 32'd100); // Source address (100 * 4)
        cpu_write(4'd1, 32'd200); // Destination address (200 * 4)
        cpu_write(4'd3, 32'd4);   // Number of words to copy
        cpu_write(4'd0, 32'd1);   // Start the copy
		  cpu_read(4'd0);

        wait(~slave_waitrequest);
		  #200;
        // Verify the copied data
        if (memory[200] !== 32'hDEADBEEF) $error("Data mismatch at address 200");
        if (memory[204] !== 32'hCAFEBABE) $error("Data mismatch at address 201");
        if (memory[208] !== 32'h12345678) $error("Data mismatch at address 202");
        if (memory[212] !== 32'h87654321) $error("Data mismatch at address 203");

        $display("Copy operation successful!");

        // Finish simulation
        $stop;
    end
endmodule
