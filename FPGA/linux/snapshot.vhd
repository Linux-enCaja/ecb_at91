library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity snapshot is
	port (
		CLKIN		: in	std_logic;

		MCLKOUT		: out	std_logic;
		MCLKIN		: in	std_logic;
		MFRAME		: in	std_logic;
		MLINE		: in	std_logic;
		MD		: in	std_logic_vector( 9 downto 0);

		SCLKOUT		: out	std_logic;
		SA		: out	std_logic_vector(12 downto 0);
		SBA		: out	std_logic_vector( 1 downto 0);
		SCS		: out	std_logic;
		SCKE		: out	std_logic;
		SRAS		: out	std_logic;
		SCAS		: out	std_logic;
		SWE		: out	std_logic;
		SDQ		: inout	std_logic_vector(15 downto 0);
		SDQM		: out	std_logic_vector( 1 downto 0);
		
		NRESET		: in	std_logic;
		NCS		: in	std_logic;
		NWE		: in	std_logic;
		NRD		: in	std_logic;
		DATA		: inout	std_logic_vector (7 downto 0)
	);
end snapshot;

architecture RTL of snapshot is

component sdr_sdram
	generic (
		ASIZE		: integer := 24;
		DSIZE		: integer := 16;
		ROWSIZE		: integer := 13;
		COLSIZE		: integer := 9;
		BANKSIZE	: integer := 2;
		ROWSTART	: integer := 9;
		COLSTART	: integer := 0;
		BANKSTART	: integer := 22
	);
	port (
		CLK		: in	std_logic;				--System Clock
		RESET_N		: in	std_logic;				--System Reset
		ADDR		: in	std_logic_vector(ASIZE-1 downto 0);	--Address for controller requests
		CMD		: in	std_logic_vector(2 downto 0);		--Controller command 
		CMDACK		: out	std_logic;				--Controller command acknowledgement
		DATAIN		: in	std_logic_vector(DSIZE-1 downto 0);	--Data input
		DATAOUT		: out	std_logic_vector(DSIZE-1 downto 0);	--Data output
		DM		: in	std_logic_vector(DSIZE/8-1 downto 0);	--Data mask input
		SA		: out	std_logic_vector(12 downto 0);		--SDRAM address output
		BA		: out	std_logic_vector(1 downto 0);		--SDRAM bank address
		CS_N		: out	std_logic;				--SDRAM Chip Selects
		CKE		: out	std_logic;				--SDRAM clock enable
		RAS_N		: out	std_logic;				--SDRAM Row address Strobe
		CAS_N		: out	std_logic;				--SDRAM Column address Strobe
		WE_N		: out	std_logic;				--SDRAM write enable
		DQ		: inout	std_logic_vector(DSIZE-1 downto 0);	--SDRAM data bus
		DQM		: out	std_logic_vector(DSIZE/8-1 downto 0)	--SDRAM data mask lines
	);
end component;

component sram512x8
	PORT
	(
		data		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
		wren		: IN STD_LOGIC  := '1';
		wraddress	: IN STD_LOGIC_VECTOR (8 DOWNTO 0);
		rdaddress	: IN STD_LOGIC_VECTOR (8 DOWNTO 0);
		wrclock		: IN STD_LOGIC ;
		rdclock		: IN STD_LOGIC ;
		q		: OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
	);
end component;

component pll1
	PORT
	(
		inclk0		: IN STD_LOGIC := '0';	--  45 MHz
		c0		: OUT STD_LOGIC;	--  90 MHz
		c1		: OUT STD_LOGIC		-- 180 MHz
	);
end component;

	signal	CLK45  : std_logic;
	signal	CLK90  : std_logic;
	signal	CLK180 : std_logic;

	signal	SRAM1_DATAIN	: std_logic_vector (7 downto 0);
	signal	SRAM1_RADDR	: std_logic_vector (8 downto 0);
	signal	SRAM1_WADDR	: std_logic_vector (8 downto 0);
	signal	SRAM1_WRENA	: std_logic;
	signal	SRAM1_WRENB	: std_logic;
	signal	SRAM1_WRSEL	: std_logic;
	signal	SRAM1_WRSELsync	: std_logic;
	signal	SRAM1_RDSEL	: std_logic;
	signal	SRAM1_DATAOUT	: std_logic_vector (7 downto 0);
	signal	SRAM1_DATAOUTA	: std_logic_vector (7 downto 0);
	signal	SRAM1_DATAOUTB	: std_logic_vector (7 downto 0);

	signal	SRAM2_DATAIN	: std_logic_vector (7 downto 0);
	signal	SRAM2_RADDR	: std_logic_vector (8 downto 0);
	signal	SRAM2_WADDR	: std_logic_vector (8 downto 0);
	signal	SRAM2_WRENA	: std_logic;
	signal	SRAM2_WRENB	: std_logic;
	signal	SRAM2_WRSEL	: std_logic;
	signal	SRAM2_RDSEL	: std_logic;
	signal	SRAM2_RDSELsync	: std_logic;
	signal	SRAM2_DATAOUT	: std_logic_vector (7 downto 0);
	signal	SRAM2_DATAOUTA	: std_logic_vector (7 downto 0);
	signal	SRAM2_DATAOUTB	: std_logic_vector (7 downto 0);

	signal	SDRAM_ADDR	: std_logic_vector (23 downto 0);
	signal	SDRAM_CMD	: std_logic_vector ( 2 downto 0);
	signal	SDRAM_CMDACK	: std_logic;
	signal	SDRAM_DATAIN	: std_logic_vector (15 downto 0);
	signal	SDRAM_DATAOUT	: std_logic_vector (15 downto 0);

	type	SDRAM_STATE_T is (SD_WAIT100, SD_PRECHARGE, SD_REFRESH1, SD_REFRESH2, SD_LOADMODE,
				  SD_WRFRAME, SD_RDFRAME, SD_REFRESH_WR, SD_REFRESH_RD);
	signal	SDRAM_STATE : SDRAM_STATE_T;
	
	type	FRAME_STATE_T is (FRAME_WAIT, FRAME_COPY, FRAME_DONE);
	signal	FRAME_STATE : FRAME_STATE_T;
begin
	SRAM1_DATAOUT <= SRAM1_DATAOUTA when SRAM1_RDSEL = '0' else SRAM1_DATAOUTB;
	SRAM2_DATAOUT <= SRAM2_DATAOUTA when SRAM2_RDSEL = '0' else SRAM2_DATAOUTB;
	SDRAM_DATAIN  <= "00000000" & SRAM1_DATAOUT;

	DATA <= SRAM2_DATAOUT when NRD = '0' else (others => 'Z');
	
	MCLKOUT <= CLK45;
	SCLKOUT <= CLK90;

sdram: sdr_sdram
	generic map (
		ASIZE		=> 24,
		DSIZE		=> 16,
		ROWSIZE		=> 13,
		COLSIZE		=> 9,
		BANKSIZE	=> 2,
		ROWSTART	=> 9,
		COLSTART	=> 0,
		BANKSTART	=> 21
	)
	port map (
		CLK		=> CLK90,
		RESET_N		=> NRESET,
		ADDR		=> SDRAM_ADDR,
		CMD		=> SDRAM_CMD,
		CMDACK		=> SDRAM_CMDACK,
		DATAIN		=> SDRAM_DATAIN,
		DATAOUT		=> SDRAM_DATAOUT,
		DM		=> "00",
		SA		=> SA,
		BA		=> SBA,
		CS_N		=> SCS,
		CKE		=> SCKE,
		RAS_N		=> SRAS,
		CAS_N		=> SCAS,
		WE_N		=> SWE,
		DQ		=> SDQ,
		DQM		=> SDQM
	);

sram1a: sram512x8
	port map (
		data		=> SRAM1_DATAIN,
		wren		=> SRAM1_WRENA,
		wraddress	=> SRAM1_WADDR,
		rdaddress	=> SRAM1_RADDR,
		wrclock		=> MCLKIN,
		rdclock		=> CLK90,
		q		=> SRAM1_DATAOUTA
	);

sram1b: sram512x8
	port map (
		data		=> SRAM1_DATAIN,
		wren		=> SRAM1_WRENB,
		wraddress	=> SRAM1_WADDR,
		rdaddress	=> SRAM1_RADDR,
		wrclock		=> MCLKIN,
		rdclock		=> CLK90,
		q		=> SRAM1_DATAOUTB
	);

sram2a: sram512x8
	port map (
		data		=> SRAM2_DATAIN,
		wren		=> SRAM2_WRENA,
		wraddress	=> SRAM2_WADDR,
		rdaddress	=> SRAM2_RADDR,
		wrclock		=> CLK90,
		rdclock		=> CLK180,
		q		=> SRAM2_DATAOUTA
	);

sram2b: sram512x8
	port map (
		data		=> SRAM2_DATAIN,
		wren		=> SRAM2_WRENB,
		wraddress	=> SRAM2_WADDR,
		rdaddress	=> SRAM2_RADDR,
		wrclock		=> CLK90,
		rdclock		=> CLK180,
		q		=> SRAM2_DATAOUTB
	);

pll: pll1
	port map (
		inclk0		=> CLKIN,
		c0		=> CLK90,
		c1		=> CLK180
	);

process(NRESET, CLK90)
begin
	if NRESET = '0' then
		CLK45 <= '0';
	else
		if rising_edge(CLK90) then
			CLK45 <= not CLK45;
		end if;
	end if;
end process;

process(NRESET, MCLKIN, MFRAME, MLINE, SDRAM_STATE, FRAME_STATE)
	variable waddr : std_logic_vector (8 downto 0);
	variable wrsel : std_logic;
begin
	if NRESET = '0' then
		waddr       := (others => '0');
		SRAM1_WADDR <= (others => '0');
		wrsel       := '0';
		SRAM1_WRSEL <= '0';
		SRAM1_WRENA <= '0';
		SRAM1_WRENB <= '0';

		FRAME_STATE <= FRAME_WAIT;
	else
		if ((SDRAM_STATE = SD_WRFRAME) or (SDRAM_STATE = SD_REFRESH_WR)) then
			if (MFRAME = '0') and (FRAME_STATE = FRAME_COPY) then
				FRAME_STATE <= FRAME_DONE;
			elsif rising_edge(MFRAME) and (FRAME_STATE = FRAME_WAIT) then
				FRAME_STATE <= FRAME_COPY;
			end if;
		end if;

		if falling_edge(MCLKIN) then
			if (FRAME_STATE = FRAME_COPY) and (MLINE = '1') then
				SRAM1_WRSEL  <= wrsel;
				SRAM1_WRENA  <= not wrsel;
				SRAM1_WRENB  <=     wrsel;

				SRAM1_WADDR  <= waddr;
				SRAM1_DATAIN <= MD(9 downto 2);
--				SRAM1_DATAIN <= waddr(7 downto 0);

				if waddr = "111111111" then
					wrsel := not wrsel;
				end if;
				waddr := waddr + 1;
			else
				SRAM1_WRENA <= '0';
				SRAM1_WRENB <= '0';
			end if;
		end if;
	end if;
end process;

process(NRESET, CLK90)
begin
	if NRESET = '0' then
		SRAM1_WRSELsync <= '0';
		SRAM2_RDSELsync <= '0';
	else
		if rising_edge(CLK90) then
			SRAM1_WRSELsync <= SRAM1_WRSEL;
			SRAM2_RDSELsync <= SRAM2_RDSEL;
		end if;
	end if;
end process;

process(NRESET, CLK90)
	variable saddr : std_logic_vector (23 downto 0);
	variable baddr : std_logic_vector ( 8 downto 0);

	variable waitcount : integer;
	variable rwcycle   : integer;
	variable readinit  : std_logic;
begin
	if NRESET = '0' then
		saddr     := (others => '0');
		baddr     := (others => '0');
		readinit  := '0';
		rwcycle   := 0;
		waitcount := 0;

		SRAM1_RADDR <= (others => '0');
		SRAM2_WADDR <= (others => '0');
		SRAM1_RDSEL <= '0';
		SRAM2_WRSEL <= '0';

		SDRAM_STATE <= SD_WAIT100;
	elsif rising_edge(CLK90) then
		case SDRAM_STATE is
			when SD_WAIT100 =>					-- wait 100 us
				SDRAM_CMD <= "000";
				if waitcount > 100 then
					SDRAM_STATE <= SD_PRECHARGE;
					SDRAM_CMD   <= "000";
				else
					waitcount := waitcount + 1;
				end if;
			when SD_PRECHARGE =>
				if SDRAM_CMDACK = '1' then
					SDRAM_STATE <= SD_REFRESH1;
					SDRAM_CMD   <= "000";
				else
					SDRAM_CMD   <= "100";		-- PRECHARGE
				end if;
			when SD_REFRESH1 =>
				if SDRAM_CMDACK = '1' then
					SDRAM_STATE <= SD_REFRESH2;
					SDRAM_CMD   <= "000";
				else
					SDRAM_CMD   <= "011";		-- REFRESH
				end if;		
			when SD_REFRESH2 =>
				if SDRAM_CMDACK = '1' then
					SDRAM_STATE <= SD_LOADMODE;
					SDRAM_CMD   <= "000";
				else
					SDRAM_CMD   <= "011";		-- REFRESH
				end if;		
			when SD_LOADMODE =>
				if SDRAM_CMDACK = '1' then
					SDRAM_STATE <= SD_WRFRAME;
					SDRAM_CMD   <= "000";
				else
					SDRAM_ADDR( 2 downto 0)  <= "111";	-- BL = Full Page
					SDRAM_ADDR( 3)           <= '0';	-- Sequential
					SDRAM_ADDR( 6 downto 4)  <= "010";	-- CL
					SDRAM_ADDR( 8 downto 7)  <= "00";	-- Standard mode
					SDRAM_ADDR(9)		 <= '0';	-- Programmed burst length
					SDRAM_ADDR(12 downto 10) <= "000";	-- (reserved)
					
					SDRAM_CMD <= "101";		-- LOAD_MODE
				end if;
			when SD_WRFRAME =>
				if (SRAM1_WRSELsync /= SRAM1_RDSEL) then
					if rwcycle = 0 then
						SDRAM_CMD   <= "010";		-- WRITEA
						SDRAM_ADDR  <= saddr;
						SRAM1_RADDR <= baddr;
						baddr   := baddr + 1;
						saddr   := saddr + 512;
						rwcycle := 1;
					elsif rwcycle = 1 then
						if SDRAM_CMDACK = '1' then
							SDRAM_CMD   <= "000";
							SRAM1_RADDR <= baddr;
							baddr   := baddr + 1;
							rwcycle := 2;
						end if;
					else
						if rwcycle = (2 + 512 - 4) then
							SDRAM_CMD   <= "100";
						end if;				

						if SDRAM_CMDACK = '1' then
							SDRAM_CMD   <= "000";
							SRAM1_RDSEL <= not SRAM1_RDSEL;
							SDRAM_STATE <= SD_REFRESH_WR;
							baddr   := (others => '0');
							rwcycle := 0;
						else
							SRAM1_RADDR <= baddr;
							baddr   := baddr   + 1;
							rwcycle := rwcycle + 1;
						end if;
					end if;
				else
					if FRAME_STATE = FRAME_DONE then
						SDRAM_STATE <= SD_RDFRAME;
						saddr := (others => '0');
					else
						SDRAM_STATE <= SD_REFRESH_WR;
					end if;
				end if;
			when SD_REFRESH_WR =>
				if SDRAM_CMDACK = '1' then
					SDRAM_STATE <= SD_WRFRAME;
					SDRAM_CMD   <= "000";
				else
					SDRAM_CMD   <= "011";		-- REFRESH
				end if;
			when SD_RDFRAME =>
				if (SRAM2_WRSEL /= SRAM2_RDSELsync) or (readinit = '0') then
					if rwcycle = 0 then
						SDRAM_CMD  <= "001";		-- READA
						SDRAM_ADDR <= saddr;
						saddr   := saddr + 512;
						rwcycle := 1;
					elsif rwcycle = 1 then
						if SDRAM_CMDACK = '1' then
							SDRAM_CMD <= "000";
							rwcycle := 2;
						end if;
					elsif rwcycle < 7 then
						rwcycle := rwcycle + 1;
					elsif rwcycle < (7 + 512) then
						if rwcycle = (7 + 512 - 9) then
							SDRAM_CMD <= "100";
						end if;
						
						if SDRAM_CMDACK = '1' then
							SDRAM_CMD <= "000";
						end if;

						SRAM2_WADDR  <= baddr;
						SRAM2_DATAIN <= SDRAM_DATAOUT(7 downto 0);
						SRAM2_WRENA  <= not SRAM2_WRSEL;
						SRAM2_WRENB  <=     SRAM2_WRSEL;
						baddr   := baddr + 1;
						rwcycle := rwcycle + 1;
					else
						SRAM2_WRSEL <= not SRAM2_WRSEL;
						SRAM2_WRENA <= '0';
						SRAM2_WRENB <= '0';
						SDRAM_STATE <= SD_REFRESH_RD;
						baddr    := (others => '0');
						readinit := '1';
						rwcycle  := 0;
					end if;
				else
					SDRAM_STATE <= SD_REFRESH_RD;
				end if;
			when SD_REFRESH_RD =>
				if SDRAM_CMDACK = '1' then
					SDRAM_STATE <= SD_RDFRAME;
					SDRAM_CMD   <= "000";
				else
					SDRAM_CMD   <= "011";		-- REFRESH
				end if;
			when others =>
				null;
		end case;
	end if;
end process;

process(NRESET, NCS, NRD, NWE)
	variable raddr : std_logic_vector (8 downto 0);
	variable rdsel : std_logic;
begin
	if NRESET = '0' then
		raddr       := (others => '0');
		SRAM2_RADDR <= (others => '0');
		rdsel       := '0';
		SRAM2_RDSEL <= '0';
	else
		if rising_edge(NRD) then
			if SRAM2_RADDR = "111111111" then
				rdsel := not rdsel;
			end if;

			raddr := raddr + 1;
			SRAM2_RDSEL <= rdsel;
			SRAM2_RADDR <= raddr;
		end if;
	end if;
end process;

end RTL;