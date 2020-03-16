<?php  
//Connect to database
require 'connectDB.php';
date_default_timezone_set('Asia/Damascus');
$d = date("Y-m-d");
$t = date("H:i:sa");

if (isset($_GET['FingerID']) && isset($_GET['device_token'])) {
    
    $fingerID = $_GET['FingerID'];
    $device_uid = $_GET['device_token'];

    $sql = "SELECT * FROM devices WHERE device_uid=?";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select_device";
        exit();
    }
    else{
        mysqli_stmt_bind_param($result, "s", $device_uid);
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)){
            $device_mode = $row['device_mode'];
            $device_dep = $row['device_dep'];
            if ($device_mode == 1) {
                $sql = "SELECT * FROM users WHERE fingerprint_id=?";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select_card";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $fingerID);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    if ($row = mysqli_fetch_assoc($resultl)){
                        //*****************************************************
                        //An existed fingerprint has been detected for Login or Logout
                        if ($row['username'] != "None" && $row['add_fingerid'] == 0){
                            $Uname = $row['username'];
                            $Number = $row['serialnumber'];
                            $sql = "SELECT * FROM users_logs WHERE fingerprint_id=? AND checkindate=? AND timeout=''";
                            $result = mysqli_stmt_init($conn);
                            if (!mysqli_stmt_prepare($result, $sql)) {
                                echo "SQL_Error_Select_logs";
                                exit();
                            }
                            else{
                                mysqli_stmt_bind_param($result, "ss", $fingerID, $d);
                                mysqli_stmt_execute($result);
                                $resultl = mysqli_stmt_get_result($result);
                                //*****************************************************
                                //Login
                                if (!$row = mysqli_fetch_assoc($resultl)){

                                    $sql = "INSERT INTO users_logs (username, serialnumber, fingerprint_id, device_uid, device_dep,checkindate, timein, timeout) VALUES (? ,?, ?, ?, ?, ?, ?, ?)";
                                    $result = mysqli_stmt_init($conn);
                                    if (!mysqli_stmt_prepare($result, $sql)) {
                                        echo "SQL_Error_Select_login1";
                                        exit();
                                    }
                                    else{
                                        $timeout = "00:00:00";
                                        mysqli_stmt_bind_param($result, "sdisssss", $Uname, $Number, $fingerID, $device_uid, $device_dep, $d, $t, $timeout);
                                        mysqli_stmt_execute($result);

                                        echo "login".$Uname;
                                        exit();
                                    }
                                }
                                //*****************************************************
                                //Logout
                                else{
                                    $sql="UPDATE users_logs SET timeout=?, fingerout=1 WHERE fingerprint_id=? AND checkindate=? AND fingerout=0";
                                    $result = mysqli_stmt_init($conn);
                                    if (!mysqli_stmt_prepare($result, $sql)) {
                                        echo "SQL_Error_insert_logout1";
                                        exit();
                                    }
                                    else{
                                        mysqli_stmt_bind_param($result, "sis", $t, $fingerID, $d);
                                        mysqli_stmt_execute($result);

                                        echo "logout".$Uname;
                                        exit();
                                    }
                                }
                            }
                        }
                        else{
                            echo "Not registerd!";
                            exit();
                        }
                    }
                    else{
                        echo "Not found!";
                        exit();
                    }
                }
            }
            else if ($device_mode == 0) {
                //New Fingerprint has been added
                $sql = "SELECT * FROM users WHERE fingerprint_id=? AND device_uid=?";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select_card";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "ss", $fingerID, $device_uid);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    if ($row = mysqli_fetch_assoc($resultl)){
                        echo "available";
                        exit();
                    }
                    else{
                        $sql = "INSERT INTO users ( device_uid, device_dep, fingerprint_id, user_date, add_fingerid) VALUES (?, ?, ?, CURDATE(), 0)";
                        $result = mysqli_stmt_init($conn);
                        if (!mysqli_stmt_prepare($result, $sql)) {
                            echo "SQL_Error_Select_add";
                            exit();
                        }
                        else{
                            mysqli_stmt_bind_param($result, "sss", $device_uid, $device_dep, $fingerID );
                            mysqli_stmt_execute($result);

                            echo "succesful";
                            exit();
                        }
                    }
                }    
            }
        }
        else{
            echo "Invalid Device!";
            exit();
        }
    }          
}
if (isset($_GET['Get_Fingerid']) && isset($_GET['device_token'])) {
    
    $device_uid = $_GET['device_token'];

    $sql = "SELECT * FROM devices WHERE device_uid=?";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select_device";
        exit();
    }
    else{
        mysqli_stmt_bind_param($result, "s", $device_uid);
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)){

            if ($_GET['Get_Fingerid'] == "get_id") {
                $sql= "SELECT fingerprint_id FROM users WHERE add_fingerid=1 AND device_uid=? LIMIT 1";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $device_uid);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    if ($row = mysqli_fetch_assoc($resultl)) {
                        echo "add-id".$row['fingerprint_id'];
                        exit();
                    }
                    else{
                        echo "Nothing";
                        exit();
                    }
                }
            }
            else{
                exit();
            }
        }
        else{
            echo "Invalid Device";
            exit();
        }
    }
}
if (isset($_GET['Check_mode']) && isset($_GET['device_token'])) {
    
    $device_uid = $_GET['device_token'];

    $sql = "SELECT * FROM devices WHERE device_uid=?";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select_device";
        exit();
    }
    else{
        mysqli_stmt_bind_param($result, "s", $device_uid);
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)){
            if ($_GET['Check_mode'] == "get_mode") {
                $sql= "SELECT device_mode FROM devices WHERE device_uid=?";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $device_uid);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    if ($row = mysqli_fetch_assoc($resultl)) {
                        echo "mode".$row['device_mode'];
                        exit();
                    }
                    else{
                        echo "Nothing";
                        exit();
                    }
                }
            }
            else{
                exit();
            }
        }
        else{
            echo "Invalid Device";
            exit();
        }
    }  
}
if (!empty($_GET['confirm_id']) && isset($_GET['device_token'])) {

    $fingerid = $_GET['confirm_id'];
    $device_uid = $_GET['device_token'];

    $sql = "SELECT * FROM devices WHERE device_uid=?";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select_device";
        exit();
    }
    else{
        mysqli_stmt_bind_param($result, "s", $device_uid);
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)){

            $sql="UPDATE users SET fingerprint_select=0 WHERE fingerprint_select=1 AND device_uid=?";
            $result = mysqli_stmt_init($conn);
            if (!mysqli_stmt_prepare($result, $sql)) {
                echo "SQL_Error_Select";
                exit();
            }
            else{
                mysqli_stmt_bind_param($result, "s", $device_uid);
                mysqli_stmt_execute($result);
                
                $sql="UPDATE users SET add_fingerid=0, fingerprint_select=1 WHERE fingerprint_id=? AND device_uid=?";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "ss", $fingerid, $device_uid);
                    mysqli_stmt_execute($result);
                    echo "Fingerprint has been added!";
                    exit();
                }
            }  
        }
        else{
            echo "Invalid Device";
            exit();
        }
    } 
}
if (isset($_GET['DeleteID']) && isset($_GET['device_token'])) {

    $device_uid = $_GET['device_token'];
    if ($_GET['DeleteID'] == "check") {
        $sql = "SELECT * FROM devices WHERE device_uid=?";
        $result = mysqli_stmt_init($conn);
        if (!mysqli_stmt_prepare($result, $sql)) {
            echo "SQL_Error_Select_device";
            exit();
        }
        else{
            mysqli_stmt_bind_param($result, "s", $device_uid);
            mysqli_stmt_execute($result);
            $resultl = mysqli_stmt_get_result($result);
            if ($row = mysqli_fetch_assoc($resultl)){
                $sql = "SELECT fingerprint_id FROM users WHERE del_fingerid=1 AND device_uid=? LIMIT 1";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $device_uid);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    if ($row = mysqli_fetch_assoc($resultl)) {
                        
                        echo "del-id".$row['fingerprint_id'];

                        $sql = "DELETE FROM users WHERE del_fingerid=1";
                        $result = mysqli_stmt_init($conn);
                        if (!mysqli_stmt_prepare($result, $sql)) {
                            echo "SQL_Error_delete";
                            exit();
                        }
                        else{
                            mysqli_stmt_execute($result);
                            exit();
                        }
                    }
                    else{
                        echo "nothing";
                        exit();
                    }
                }
            }
            else{
                echo "Invalid Device";
                exit();
            }
        }
    }
    else{
        exit();
    }
}
?>
