/*
 *  Macro to run the online for all the detectors simultaneously
 *
 *  One needs to set up the 2020 experiments: s444 or s467, the unpackers are:
 *
 *  at $UCESB_DIR/../upexps/202002_s444 and $UCESB_DIR/../upexps/202002_s467
 *
 *
 *  Author: Jose Luis <joseluis.rodriguez.sanchez@usc.es>
 *  @since Feb 20th, 2020
 *
 */

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_TPAT_t unpacktpat;
    EXT_STR_h101_SOFMWPC_onion_t mwpc;
    EXT_STR_h101_MUSIC_onion_t music;
    EXT_STR_h101_SOFSCI_onion_t sci;
    EXT_STR_h101_AMS_t ams;
    EXT_STR_h101_WRMASTER_t wrmaster;
    EXT_STR_h101_WRSOFIA_t wrsofia;
    EXT_STR_h101_WRS2_t wrs2;
    EXT_STR_h101_WRS8_t wrs8;
    EXT_STR_h101_CALIFA_t califa;
    EXT_STR_h101_WRCALIFA_t wrcalifa;
    EXT_STR_h101_SOFTWIM_onion_t twim;
    EXT_STR_h101_SOFTOFW_onion_t tofw;
    EXT_STR_h101_SOFSCALERS_onion_t scalers;
    EXT_STR_h101_raw_nnp_tamex_t raw_nnp;
    EXT_STR_h101_WRNEULAND_t wrneuland;
    EXT_STR_h101_FRS_t frs;
} EXT_STR_h101;

void nearline(int runnum, TString settingname, int port);
void nearline(int runnum, TString settingname){
  nearline( runnum, settingname, 8090);
}
void nearline(int runnum){
  nearline(runnum, "50Ca");
}
void nearline(){
  nearline(301, "50Ca");
}

void nearline(int runnum, TString settingname, int port)
{
    TStopwatch timer;
    timer.Start();

    // const Int_t nev = -1; // number of events to read, -1 - until CTRL+C
    const Int_t nev = -1; // Only nev events to read
    const Int_t fRunId = 1;

    // *********************************** //
    // PLEASE CHANGE THE EXPERIMENT NUMBER //
    // *********************************** //
    const Int_t expId = 467;               // select experiment: 444 or 467
    // *********************************** //
    
    // NumSofSci, file names and paths -----------------------------
    Int_t sofiaWR, NumSofSci, IdS2, IdS8;
    TString dir = gSystem->Getenv("VMCWORKDIR");
    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");
    TString filename, outputFilename, upexps_dir, ucesb_path, sofiacaldir;

    if(runnum==0){
      //filename = "--stream=lxlanddaq01:9000";
      cerr<<"No online analysis available"<<endl;
      return 1;
    }else if (expId==444){ // not modified
      NumSofSci = 1; // s444: PRIMARY BEAM EXP, 1 SofSci at CAVE C ONLY
      IdS2 = 0;
      IdS8 = 0;
      sofiaWR = 0x500;
      
      //filename = "--stream=lxir123:7803";
      filename = "/lustre/land/202002_s444/stitched/main0040_0001.lmd";
      outputFilename = "data_s444_online.root";
      
      upexps_dir = ucesb_dir + "/../upexps/";                      // for local computers
      // upexps_dir = "/u/land/fake_cvmfs/upexps";                 // for lxlandana computers
      // upexps_dir = "/u/land/lynx.landexp/202002_s444/upexps/";  // for lxg computers
      ucesb_path = upexps_dir + "/202002_s444/202002_s444 --allow-errors --input-buffer=100Mi";
      
      sofiacaldir = dir + "/sofia/macros/s444/parameters/";
    }
    else if (expId==467){
      NumSofSci = 4; // s467: SECONDARY BEAM EXP, 2 at S2, 1 at S8, 1 at CAVE C
      IdS2 = 2;
      IdS8 = 3;
      sofiaWR = 0xe00;
      
      //filename = "/lustre/land/202002_s444/lustre/r3b/202002_s444/main0007_0001.lmd";
      filename = Form("/u/taniuchi/s467/s467_lustertmp/main%04d_*.lmd",runnum);
      outputFilename = Form("./rootfiles_local/data_s467_%04d.root",runnum);
    
      upexps_dir = ucesb_dir + "/../upexps/";                      // for local computers // copied from fake and recompiled
      // upexps_dir = "/u/land/fake_cvmfs/upexps";                 // for lxlandana computers
      // upexps_dir = "/u/land/lynx.landexp/202002_s467/upexps/";  // for lxg computers
      ucesb_path = upexps_dir + "/202002_s467/202002_s467 --allow-errors --input-buffer=100Mi";

      sofiacaldir = dir + "/sofia/macros/s467_ryotani/parameters/";
    }
    else{
      std::cout << "Experiment was not selected" << std::endl;
      gApplication->Terminate();
    }
    // Output file -----------------------------------------
    //TString sofiacalfilename = sofiacaldir + "CalibParam_"+ settingname +".par";
    TString sofiacalfilename = sofiacaldir + "CalibParam.par";
    //cout<<sofiacalfilename<<endl;
    ucesb_path.ReplaceAll("//", "/");
    sofiacalfilename.ReplaceAll("//", "/");
        
    // store data or not ------------------------------------
    Bool_t fCal_level_califa = true;  // set true if there exists a file with the calibration parameters
    Bool_t NOTstoremappeddata = true; // if true, don't store mapped data in the root file
    Bool_t NOTstorecaldata = true;    // if true, don't store cal data in the root file
    Bool_t NOTstorehitdata = true;    // if true, don't store hit data in the root file
    
    // Online server configuration --------------------------
    Int_t refresh = 1; // Refresh rate for online histograms
    //Int_t port = 9090; // Port number for the online visualization, example lxgXXXX:8888

    // Setup: Selection of detectors ------------------------
    Bool_t fFrs = false;     // FRS for production of exotic beams (just scintillators)
    Bool_t fFrsTpcs = false; // Tpcs at FRS (S2) for scintillator calibration in position
    Bool_t fFrsMws = false;  // MWs at FRS (S8) for beam position
    Bool_t fFrsSci = true;   // Start: Plastic scintillators at FRS
    Bool_t fMwpc0 = true;    // MWPC0 for tracking at entrance of Cave-C
    Bool_t fMusic = true;    // R3B-Music: Ionization chamber for charge-Z
    Bool_t fSci = true;      // Start: Plastic scintillator for ToF
    Bool_t fAms = false;     // AMS tracking detectors
    Bool_t fCalifa = false;  // Califa calorimeter
    Bool_t fMwpc1 = true;    // MWPC1 for tracking of fragments in front of target
    Bool_t fMwpc2 = true;    // MWPC2 for tracking of fragments before GLAD
    Bool_t fTwim = true;     // Twim: Ionization chamber for charge-Z of fragments
    Bool_t fMwpc3 = true;    // MWPC3 for tracking of fragments behind GLAD
    Bool_t fTofW = true;     // ToF-Wall for time-of-flight of fragments behind GLAD
    Bool_t fScalers = true;  // SIS3820 scalers at Cave C
    Bool_t fNeuland = false;  // NeuLAND for neutrons behind GLAD
    Bool_t fTracking = true; // Tracking of fragments inside GLAD

    // Calibration files ------------------------------------
    // Parameters for CALIFA mapping
    TString califamapdir = dir + "/macros/r3b/unpack/s467/califa/parameters/";
    TString califamapfilename = califamapdir + "CALIFA_mapping.par";
    califamapfilename.ReplaceAll("//", "/");
    // Parameters for CALIFA calibration in keV
    TString califadir = dir + "/macros/r3b/unpack/s467/califa/parameters/";
    TString califacalfilename = califadir + "Califa_Cal8Feb2020.root";
    califacalfilename.ReplaceAll("//", "/");

    // Create source using ucesb for input ------------------
    EXT_STR_h101 ucesb_struct;

    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);
    
    // Definition of reader ---------------------------------
    R3BUnpackReader* unpackreader =
        new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct, offsetof(EXT_STR_h101, unpack));
    R3BTrloiiTpatReader* unpacktpat =
        new R3BTrloiiTpatReader((EXT_STR_h101_TPAT*)&ucesb_struct, offsetof(EXT_STR_h101, unpacktpat));

    R3BFrsReaderNov19* unpackfrs;
    R3BMusicReader* unpackmusic;
    R3BSofSciReader* unpacksci;
    R3BWhiterabbitS2Reader* unpackWRS2;
    R3BWhiterabbitS8Reader* unpackWRS8;
    R3BWhiterabbitMasterReader* unpackWRMaster;
    R3BSofWhiterabbitReader* unpackWRSofia;
    R3BAmsReader* unpackams;
    R3BCalifaFebexReader* unpackcalifa;
    R3BWhiterabbitCalifaReader* unpackWRCalifa;
    R3BSofMwpcReader* unpackmwpc;
    R3BSofTwimReader* unpacktwim;
    R3BSofTofWReader* unpacktofw;
    R3BSofScalersReader* unpackscalers;
    R3BNeulandTamexReader* unpackneuland;
    R3BWhiterabbitNeulandReader* unpackWRNeuland;

    
    if (fFrsTpcs)
      unpackfrs= new R3BFrsReaderNov19((EXT_STR_h101_FRS*)&ucesb_struct.frs,
					     offsetof(EXT_STR_h101, frs));
    
    if (fMusic)
        unpackmusic = new R3BMusicReader((EXT_STR_h101_MUSIC_t*)&ucesb_struct.music, offsetof(EXT_STR_h101, music));
    
    if(fFrsSci) {
     unpackWRS2 = new R3BWhiterabbitS2Reader(
            (EXT_STR_h101_WRS2*)&ucesb_struct.wrs2, offsetof(EXT_STR_h101, wrs2), 0x200);
     unpackWRS8 = new R3BWhiterabbitS8Reader(
            (EXT_STR_h101_WRS8*)&ucesb_struct.wrs8, offsetof(EXT_STR_h101, wrs8), 0x800);
    }
    
    if (fSci)
    {
      unpacksci = new R3BSofSciReader((EXT_STR_h101_SOFSCI_t*)&ucesb_struct.sci, offsetof(EXT_STR_h101, sci),NumSofSci);
      unpackWRMaster = new R3BWhiterabbitMasterReader((EXT_STR_h101_WRMASTER*)&ucesb_struct.wrmaster, offsetof(EXT_STR_h101, wrmaster), 0x300);
      unpackWRSofia = new R3BSofWhiterabbitReader((EXT_STR_h101_WRSOFIA*)&ucesb_struct.wrsofia, offsetof(EXT_STR_h101, wrsofia), sofiaWR);
    }
    
    if (fAms)
        unpackams = new R3BAmsReader((EXT_STR_h101_AMS*)&ucesb_struct.ams, offsetof(EXT_STR_h101, ams));

    if (fCalifa)
    {
        unpackcalifa =
            new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa, offsetof(EXT_STR_h101, califa));
        unpackWRCalifa = new R3BWhiterabbitCalifaReader(
            (EXT_STR_h101_WRCALIFA*)&ucesb_struct.wrcalifa, offsetof(EXT_STR_h101, wrcalifa), 0xa00, 0xb00);
    }
    if (fMwpc0 || fMwpc1 || fMwpc2 || fMwpc3)
        unpackmwpc = new R3BSofMwpcReader((EXT_STR_h101_SOFMWPC_t*)&ucesb_struct.mwpc, offsetof(EXT_STR_h101, mwpc));

    if (fTwim)
        unpacktwim = new R3BSofTwimReader((EXT_STR_h101_SOFTWIM_t*)&ucesb_struct.twim, offsetof(EXT_STR_h101, twim));

    if (fTofW)
        unpacktofw = new R3BSofTofWReader((EXT_STR_h101_SOFTOFW_t*)&ucesb_struct.tofw, offsetof(EXT_STR_h101, tofw));

    if (fScalers)
        unpackscalers =
            new R3BSofScalersReader((EXT_STR_h101_SOFSCALERS_t*)&ucesb_struct.scalers, offsetof(EXT_STR_h101, scalers));

    if (fNeuland){
        //unpackneuland = new R3BNeulandTamexReader((EXT_STR_h101_raw_nnp_tamex_t*)&ucesb_struct.raw_nnp,
          //                                        offsetof(EXT_STR_h101, raw_nnp));

        unpackWRNeuland = new R3BWhiterabbitNeulandReader(
            (EXT_STR_h101_WRNEULAND*)&ucesb_struct.wrneuland, offsetof(EXT_STR_h101, wrneuland), 0x900);
    }
    
    // Add readers ------------------------------------------
    source->AddReader(unpackreader);
    source->AddReader(unpacktpat);
   
    if (fFrsTpcs)
    {
     unpackfrs->SetOnline(NOTstoremappeddata);
     source->AddReader(unpackfrs);
    }

    if (fMusic)
    {
        unpackmusic->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackmusic);
    }
    if (fSci)
    {
        unpacksci->SetOnline(NOTstoremappeddata);
        source->AddReader(unpacksci);
        unpackWRMaster->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackWRMaster);
        unpackWRSofia->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackWRSofia);
    }

    if(fFrsSci) {
        unpackWRS2->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackWRS2);
        unpackWRS8->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackWRS8);
    }
    
    if (fMwpc0 || fMwpc1 || fMwpc2 || fMwpc3)
    {
        unpackmwpc->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackmwpc);
    }
    if (fAms)
    {
        unpackams->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackams);
    }
    if (fCalifa)
    {
        unpackcalifa->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackcalifa);
        unpackWRCalifa->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackWRCalifa);
    }
    if (fTwim)
    {
        unpacktwim->SetOnline(NOTstoremappeddata);
        source->AddReader(unpacktwim);
    }
    if (fTofW)
    {
        unpacktofw->SetOnline(NOTstoremappeddata);
        source->AddReader(unpacktofw);
    }
    if (fScalers)
    {
        unpackscalers->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackscalers);
    }
    if (fNeuland)
    {
        unpackWRNeuland->SetOnline(NOTstoremappeddata);
        source->AddReader(unpackWRNeuland);
    }
    
    // Create online run ------------------------------------
    FairRunOnline* run = new FairRunOnline(source);
    run->SetRunId(fRunId);
    run->SetSink(new FairRootFileSink(outputFilename));
    run->ActivateHttpServer(refresh, port); // To show online.
    
    // Runtime data base ------------------------------------
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii
    
    if (!fCalifa)
    {
        parIo1->open(sofiacalfilename, "in");
        rtdb->setFirstInput(parIo1);
        rtdb->print();
    }
    else
    {
        if (!fCal_level_califa)
        { // SOFIA and CALIFA mapping: Ascii files
            TList* parList1 = new TList();
            parList1->Add(new TObjString(sofiacalfilename));
            parList1->Add(new TObjString(califamapfilename));
            parIo1->open(parList1);
            rtdb->setFirstInput(parIo1);
            rtdb->print();
        }
        else
        { // SOFIA, CALIFA mapping and CALIFA calibration parameters
            parIo1->open(sofiacalfilename, "in"); // Ascii file
            rtdb->setFirstInput(parIo1);
            rtdb->print();
            Bool_t kParameterMerged = kFALSE;
            FairParRootFileIo* parIo2 = new FairParRootFileIo(kParameterMerged); // Root file
            TList* parList2 = new TList();
            parList2->Add(new TObjString(califacalfilename));
            parIo2->open(parList2);
            rtdb->setSecondInput(parIo2);
        }
    }
    
    // Add analysis task ------------------------------------
    // TPCs at S2
    if (fFrsTpcs)
    {
      R3BTpcMapped2Cal* TpcMap2Cal = new R3BTpcMapped2Cal();
      TpcMap2Cal->SetOnline(NOTstorecaldata);
      run->AddTask(TpcMap2Cal);
      R3BTpcCal2Hit* TpcCal2Hit = new R3BTpcCal2Hit();
      TpcCal2Hit->SetOnline(NOTstorehitdata);
      run->AddTask(TpcCal2Hit);
    }
    // MWPC0
    if (fMwpc0)
    {
        R3BSofMwpc0Mapped2Cal* MW0Map2Cal = new R3BSofMwpc0Mapped2Cal();
        MW0Map2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(MW0Map2Cal);

        R3BSofMwpc0Cal2Hit* MW0Cal2Hit = new R3BSofMwpc0Cal2Hit();
        MW0Cal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(MW0Cal2Hit);
    }

    // MUSIC
    if (fMusic)
    {
        R3BMusicMapped2Cal* MusMap2Cal = new R3BMusicMapped2Cal();
        MusMap2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(MusMap2Cal);

        R3BMusicCal2Hit* MusCal2Hit = new R3BMusicCal2Hit();
        MusCal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(MusCal2Hit);
    }

    // SCI
    if (fSci)
    {
        // --- Mapped 2 Tcal for SofSci
        R3BSofSciMapped2Tcal* SofSciMap2Tcal = new R3BSofSciMapped2Tcal();
        SofSciMap2Tcal->SetOnline(NOTstorecaldata);
        run->AddTask(SofSciMap2Tcal);

        // --- Tcal 2 SingleTcal for SofSci
        R3BSofSciTcal2SingleTcal* SofSciTcal2STcal = new R3BSofSciTcal2SingleTcal();
        SofSciTcal2STcal->SetOnline(NOTstorecaldata);
        run->AddTask(SofSciTcal2STcal);
	
        // --- SingleTcal 2 Hit for SofSci
        R3BSofSciSingleTCal2Hit* SofSciSTcal2Hit = new R3BSofSciSingleTCal2Hit();
        SofSciSTcal2Hit->SetOnline(NOTstorehitdata);
        SofSciSTcal2Hit->SetCalParams(675.,-1922.);//ToF calibration at Cave-C
        run->AddTask(SofSciSTcal2Hit);
    }
    
    // FRS
    if (fMwpc0 && fSci && fMusic && fFrs)
    {
        R3BSofFrsAnalysis* FrsAna = new R3BSofFrsAnalysis();
        FrsAna->SetOnline(NOTstorehitdata);
        run->AddTask(FrsAna);
    }
    
    // AMS
    if (fAms)
    {
        R3BAmsMapped2StripCal* AmsMap2Cal = new R3BAmsMapped2StripCal();
        AmsMap2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(AmsMap2Cal);
        R3BAmsStripCal2Hit* AmsCal2Hit = new R3BAmsStripCal2Hit();
        AmsCal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(AmsCal2Hit);
    }

    // CALIFA
    if (fCalifa && fCal_level_califa)
    {
        // R3BCalifaMapped2CrystalCal ---
        R3BCalifaMapped2CrystalCal* CalifaMap2Cal = new R3BCalifaMapped2CrystalCal();
        CalifaMap2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(CalifaMap2Cal);
        // R3BCalifaCrystalCal2Hit ---
        R3BCalifaCrystalCal2Hit* CalifaCal2Hit = new R3BCalifaCrystalCal2Hit();
        CalifaCal2Hit->SetCrystalThreshold(100.); // 100keV
        CalifaCal2Hit->SetDRThreshold(10000.);    // 10MeV
        CalifaCal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(CalifaCal2Hit);
    }

    // MWPC1
    if (fMwpc1)
    {
        R3BSofMwpc1Mapped2Cal* MW1Map2Cal = new R3BSofMwpc1Mapped2Cal();
        MW1Map2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(MW1Map2Cal);

        R3BSofMwpc1Cal2Hit* MW1Cal2Hit = new R3BSofMwpc1Cal2Hit();
        MW1Cal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(MW1Cal2Hit);
    }

    // TWIM
    if (fTwim)
    {
        R3BSofTwimMapped2Cal* TwimMap2Cal = new R3BSofTwimMapped2Cal();
        TwimMap2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(TwimMap2Cal);

        R3BSofTwimCal2Hit* TwimCal2Hit = new R3BSofTwimCal2Hit();
        TwimCal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(TwimCal2Hit);
    }

    // MWPC2
    if (fMwpc2)
    {
        R3BSofMwpc2Mapped2Cal* MW2Map2Cal = new R3BSofMwpc2Mapped2Cal();
        MW2Map2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(MW2Map2Cal);

        R3BSofMwpc2Cal2Hit* MW2Cal2Hit = new R3BSofMwpc2Cal2Hit();
        MW2Cal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(MW2Cal2Hit);
    }

    // MWPC3
    if (fMwpc3)
    {
        R3BSofMwpc3Mapped2Cal* MW3Map2Cal = new R3BSofMwpc3Mapped2Cal();
        MW3Map2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(MW3Map2Cal);

        R3BSofMwpc3Cal2Hit* MW3Cal2Hit = new R3BSofMwpc3Cal2Hit();
        MW3Cal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(MW3Cal2Hit);
    }

    // ToF-Wall
    if (fTofW)
    {
        // --- Mapped 2 Tcal for SofTofW
        R3BSofTofWMapped2Tcal* SofTofWMap2Tcal = new R3BSofTofWMapped2Tcal();
        SofTofWMap2Tcal->SetOnline(NOTstorecaldata);
        run->AddTask(SofTofWMap2Tcal);

        // --- Tcal 2 SingleTcal for SofTofW
        R3BSofTofWTcal2SingleTcal* SofTofWTcal2STcal = new R3BSofTofWTcal2SingleTcal();
        SofTofWTcal2STcal->SetOnline(NOTstorecaldata);
        run->AddTask(SofTofWTcal2STcal);

        R3BSofTofWTCal2Hit* SofTofWTcal2Hit = new R3BSofTofWTCal2Hit();
        SofTofWTcal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(SofTofWTcal2Hit);
    }

    // Add online task ------------------------------------
    if (fFrsTpcs)
    {
       FrsTpcOnlineSpectra* tpconline= new FrsTpcOnlineSpectra();
       run->AddTask(tpconline);
    }
    
    if (fScalers)
    {
        R3BSofScalersOnlineSpectra* scalersonline = new R3BSofScalersOnlineSpectra();
        run->AddTask(scalersonline);
    }
    if (fFrs && fMusic && fSci)
    {
        R3BSofFrsOnlineSpectra* frsonline = new R3BSofFrsOnlineSpectra();
        run->AddTask(frsonline);
    }

    if (fMwpc0)
    {
        R3BSofMwpcOnlineSpectra* mw0online = new R3BSofMwpcOnlineSpectra("SofMwpc0OnlineSpectra", 1, "Mwpc0");
        run->AddTask(mw0online);
    }

    if (fMusic)
    {
        R3BMusicOnlineSpectra* musonline = new R3BMusicOnlineSpectra();
        run->AddTask(musonline);
        if (fMwpc0)
        {
          R3BSofMwpcvsMusicOnlineSpectra* mw0vsmusiconline= new R3BSofMwpcvsMusicOnlineSpectra("SofMwpc0vsMusicOnlineSpectra", 1, "Mwpc0");
          run->AddTask(mw0vsmusiconline);
        }
    }
    
    if (fSci)
    {
        R3BSofSciOnlineSpectra* scionline = new R3BSofSciOnlineSpectra();
        run->AddTask(scionline);
    }

    if (fAms)
    {
        R3BAmsOnlineSpectra* AmsOnline = new R3BAmsOnlineSpectra();
        run->AddTask(AmsOnline);
    }

    if (fCalifa)
    {
        R3BCalifaOnlineSpectra* CalifaOnline = new R3BCalifaOnlineSpectra();
        CalifaOnline->SetRange_max(30000); // 30000 -> 30MeV
        CalifaOnline->SetBinChannelFebex(500);
        CalifaOnline->SetMaxBinFebex(10000); // 10000 -> 10MeV
        run->AddTask(CalifaOnline);
    }

    if (fMusic && fCalifa && fTwim)
    {
        R3BAmsCorrelationOnlineSpectra* CalifaAmsOnline = new R3BAmsCorrelationOnlineSpectra();
        CalifaAmsOnline->SetZproj(36.);//(20.0); // Projectile atomic number
        CalifaAmsOnline->SetCalifa_bins_maxrange(500, 300000); // 300000 -> 300MeV
        run->AddTask(CalifaAmsOnline);
    }

    if (fTwim)
    {
        R3BSofTwimOnlineSpectra* twonline = new R3BSofTwimOnlineSpectra();
        run->AddTask(twonline);
        // Twim-Music correlations
        if (fMusic)
        {
            R3BSofTwimvsMusicOnlineSpectra* twmusonline = new R3BSofTwimvsMusicOnlineSpectra();
            run->AddTask(twmusonline);
        }
    }

    if (fMwpc1)
    {
        R3BSofMwpcOnlineSpectra* mw1online = new R3BSofMwpcOnlineSpectra("SofMwpc1OnlineSpectra", 1, "Mwpc1");
        run->AddTask(mw1online);
    }

    if (fMwpc0 && fMwpc1)
    {
        R3BSofMwpcCorrelationOnlineSpectra* mw0mw1online =
            new R3BSofMwpcCorrelationOnlineSpectra("SofMwpc0_1CorrelationOnlineSpectra", 1, "Mwpc0", "Mwpc1");
        run->AddTask(mw0mw1online);
    }

    if (fMwpc1 && fMwpc2)
    {
        R3BSofMwpcCorrelationOnlineSpectra* mw1mw2online =
            new R3BSofMwpcCorrelationOnlineSpectra("SofMwpc1_2CorrelationOnlineSpectra", 1, "Mwpc1", "Mwpc2");
        run->AddTask(mw1mw2online);
    }

    if (fMwpc2)
    {
        R3BSofMwpcOnlineSpectra* mw2online = new R3BSofMwpcOnlineSpectra("SofMwpc2OnlineSpectra", 1, "Mwpc2");
        run->AddTask(mw2online);
    }

    if (fMwpc0 && fMwpc2)
    {
        R3BSofMwpcCorrelationOnlineSpectra* mw0mw2online =
            new R3BSofMwpcCorrelationOnlineSpectra("SofMwpc0_2CorrelationOnlineSpectra", 1, "Mwpc0", "Mwpc2");
        run->AddTask(mw0mw2online);
    }

    if (fMwpc2 && fMwpc3)
    {
        R3BSofMwpcCorrelationOnlineSpectra* mw2mw3online =
            new R3BSofMwpcCorrelationOnlineSpectra("SofMwpc2_3CorrelationOnlineSpectra", 1, "Mwpc2", "Mwpc3");
        run->AddTask(mw2mw3online);
    }

    if (fMwpc3)
    {
        R3BSofMwpcOnlineSpectra* mw3online = new R3BSofMwpcOnlineSpectra("SofMwpc3OnlineSpectra", 1, "Mwpc3");
        run->AddTask(mw3online);
    }

    if (fTofW)
    {
        R3BSofTofWOnlineSpectra* tofwonline = new R3BSofTofWOnlineSpectra();
	//        tofwonline->Set_TwimvsTof_range(-300.,300.);
	tofwonline->Set_TwimvsTof_range(-87.,-65.);
	tofwonline->Set_IdSofSciCaveC(NumSofSci);
        run->AddTask(tofwonline);
    }
    
    if (fMwpc2 && fTwim && fSci && fTracking)
    {
        if(fTofW && fMwpc3){
         R3BSofFragmentAnalysis* TrackingAna = new R3BSofFragmentAnalysis();
         TrackingAna->SetOnline(NOTstorehitdata);
         run->AddTask(TrackingAna); 
        }

        R3BSofTrackingOnlineSpectra* Trackingonline = new R3BSofTrackingOnlineSpectra();
        Trackingonline->Set_Charge_range(10.,38.);
        run->AddTask(Trackingonline); 
    }
    R3BSofOnlineSpectra* sofonline = new R3BSofOnlineSpectra();
    run->AddTask(sofonline);
    
    // Initialize -------------------------------------------
    run->Init();
    
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    
    // Run --------------------------------------------------
    
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

    // Finish -----------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    std::cout << std::endl << std::endl;
    std::cout << "Macro finished succesfully." << std::endl;
    std::cout << "Output file is " << outputFilename << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl << std::endl;
    // gApplication->Terminate();
}

