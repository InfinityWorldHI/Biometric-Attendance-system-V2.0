<?php  
//Connect to database
require'connectDB.php';

//Add user Fingerprint
if (isset($_POST['Add_fingerID'])) {

    $fingerid = $_POST['fingerid'];
    $name = "";
    $lastname = "";
    $depart  = "";
    $email = "";
    $gender = "";
    
    if ($fingerid == 0) {
        echo "Enter a Fingerprint ID!";
        exit();
    }
    else{
        if ($fingerid > 0 && $fingerid < 128) {
            $sql = "SELECT fingerprint_id FROM teachers WHERE fingerprint_id=?";
            $result = mysqli_stmt_init($conn);
            if (!mysqli_stmt_prepare($result, $sql)) {
              echo "SQL_Error1";
              exit();
            }
            else{
                mysqli_stmt_bind_param($result, "i", $fingerid );
                mysqli_stmt_execute($result);
                $resultl = mysqli_stmt_get_result($result);
                if (!$row = mysqli_fetch_assoc($resultl)) {

                    $sql = "SELECT add_fingerid FROM teachers WHERE add_fingerid=1";
                    $result = mysqli_stmt_init($conn);
                    if (!mysqli_stmt_prepare($result, $sql)) {
                      echo "SQL_Error2";
                      exit();
                    }
                    else{
                        mysqli_stmt_execute($result);
                        $resultl = mysqli_stmt_get_result($result);
                        if (!$row = mysqli_fetch_assoc($resultl)) {
                            $sql = "INSERT INTO teachers (username, lastname, gender, department, email, fingerprint_id, add_fingerid) VALUES (?, ?, ?, ?, ?, ?, 1)";
                            $result = mysqli_stmt_init($conn);
                            if (!mysqli_stmt_prepare($result, $sql)) {
                              echo "SQL_Error3";
                              exit();
                            }
                            else{
                                mysqli_stmt_bind_param($result, "sssssi", $name, $lastname, $gender, $depart, $email, $fingerid );
                                mysqli_stmt_execute($result);
                                echo "Fingerprint ready to be scanned!";
                                exit();
                            }
                        }
                        else{
                            echo "1 Fingerprint at a time please!";
                        }
                    }   
                }
                else{
                    echo "Fingerprint already exist!";
                    exit();
                }
            }
        }
        else{
            echo "The Fingerprint ID must be between 1 & 127";
            exit();
        }
    }
}
// select Fingerprint 
if (isset($_GET['select'])) {
    header('Content-Type: application/json');
    $data = array();

    $Finger_id = $_GET['Finger_id'];

    $sql = "SELECT fingerprint_select FROM teachers WHERE fingerprint_select=1";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select";
        exit();
    }
    else{
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)) {

            $sql="UPDATE teachers SET fingerprint_select=0";
            $result = mysqli_stmt_init($conn);
            if (!mysqli_stmt_prepare($result, $sql)) {
                echo "SQL_Error_Select";
                exit();
            }
            else{
                mysqli_stmt_execute($result);

                $sql="UPDATE teachers SET fingerprint_select=1 WHERE fingerprint_id=?";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_select_Fingerprint1";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $Finger_id);
                    mysqli_stmt_execute($result);
                    $sql = "SELECT * FROM teachers WHERE fingerprint_select=1";
                    $result = mysqli_stmt_init($conn);
                    if (!mysqli_stmt_prepare($result, $sql)) {
                        echo "SQL_Error_Select";
                        exit();
                    }
                    else{
                        mysqli_stmt_execute($result);
                        $resultl = mysqli_stmt_get_result($result); 
                        if ($row = mysqli_fetch_assoc($resultl)) {
                            foreach ($resultl as $row) {
                                $data[] = $row;
                            }
                        }
                    }
                    //free memory associated with result
                    $resultl->close();

                    //close connection
                    $conn->close();

                    //now print the data
                    print json_encode($data);
                    // echo "Teacher's Fingerprint selected";
                    // exit();
                }
            }
        }
        else{
            $sql="UPDATE teachers SET fingerprint_select=1 WHERE fingerprint_id=?";
            $result = mysqli_stmt_init($conn);
            if (!mysqli_stmt_prepare($result, $sql)) {
                echo "SQL_Error_select_Fingerprint2";
                exit();
            }
            else{
                mysqli_stmt_bind_param($result, "s", $Finger_id);
                mysqli_stmt_execute($result);

                echo "Fingerprint ID selected";
                exit();
            }
        }
    } 
}
if (isset($_POST['Add'])) {
     
    $Uname = $_POST['name'];
    $lastname = $_POST['lastname'];
    $depart = $_POST['depart'];
    $Number = $_POST['number'];
    $ph_number = $_POST['ph_number'];
    $Email= $_POST['email'];


    $Gender= $_POST['gender'];

    //check if there any selected user
    $sql = "SELECT username FROM teachers WHERE fingerprint_select=1";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
      echo "SQL_Error";
      exit();
    }
    else{
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)) {

            if (empty($row['username'])) {

                if (!empty($Uname) && !empty($lastname) && !empty($Number) && !empty($Email) && $depart != "all") {
                    //check if there any user had already the Serial Number
                    $sql = "SELECT serialnumber FROM teachers WHERE serialnumber=?";
                    $result = mysqli_stmt_init($conn);
                    if (!mysqli_stmt_prepare($result, $sql)) {
                        echo "SQL_Error";
                        exit();
                    }
                    else{
                        mysqli_stmt_bind_param($result, "d", $Number);
                        mysqli_stmt_execute($result);
                        $resultl = mysqli_stmt_get_result($result);
                        if (!$row = mysqli_fetch_assoc($resultl)) {
                            $sql="UPDATE teachers SET username=?, lastname=?, serialnumber=?, gender=?, department=?, email=?, phone_number=?, user_date=CURDATE() WHERE fingerprint_select=1";
                            $result = mysqli_stmt_init($conn);
                            if (!mysqli_stmt_prepare($result, $sql)) {
                                echo "SQL_Error_select_Fingerprint3";
                                exit();
                            }
                            else{
                                mysqli_stmt_bind_param($result, "ssdsssd", $Uname, $lastname, $Number, $Gender, $depart, $Email, $ph_number);
                                mysqli_stmt_execute($result);

                                echo "A new teacher has been added!";
                                exit();
                            }
                        }
                        else {
                            echo "The serial number is already taken!";
                            exit();
                        }
                    }
                }
                else{
                    echo "Empty Fields";
                    exit();
                }
            }
            else{
                echo "This Fingerprint is already added";
                exit();
            }    
        }
        else {
            echo "There's no selected Fingerprint!";
            exit();
        }
    }
}
// Update an existing user 
if (isset($_POST['Update'])) {

    $Uname = $_POST['name'];
    $lastname = $_POST['lastname'];
    $depart = $_POST['depart'];
    $Number = $_POST['number'];
    $ph_number = $_POST['ph_number'];
    $Email= $_POST['email'];

    //optional
    $Gender= $_POST['gender'];

    if ($Number == 0) {
        $Number = -1;
    }
    //check if there any selected user
    $sql = "SELECT * FROM teachers WHERE fingerprint_select=1";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
      echo "SQL_Error";
      exit();
    }
    else{
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)) {

            if (empty($row['username'])) {
                echo "Click their Finger ID first!";
                exit();
            }
            else{
                if (empty($Uname) && empty($lastname) && empty($Number) && empty($Email)) {
                    echo "Required fields incomplete!";
                    exit();
                }
                else{
                    //check if there any user had already the Serial Number
                    $sql = "SELECT serialnumber FROM teachers WHERE serialnumber=? AND fingerprint_select=0";
                    $result = mysqli_stmt_init($conn);
                    if (!mysqli_stmt_prepare($result, $sql)) {
                        echo "SQL_Error";
                        exit();
                    }
                    else{
                        mysqli_stmt_bind_param($result, "d", $Number);
                        mysqli_stmt_execute($result);
                        $resultl = mysqli_stmt_get_result($result);
                        if (!$row = mysqli_fetch_assoc($resultl)) {

                            if (!empty($Uname) && !empty($lastname) && !empty($Email)) {

                                $sql="UPDATE teachers SET username=?, lastname=?, serialnumber=?, gender=?, department=?, email=?, phone_number=? WHERE fingerprint_select=1";
                                $result = mysqli_stmt_init($conn);
                                if (!mysqli_stmt_prepare($result, $sql)) {
                                    echo "SQL_Error_select_Fingerprint4";
                                    exit();
                                }
                                else{
                                    mysqli_stmt_bind_param($result, "ssdsssd", $Uname, $lastname, $Number, $Gender, $depart, $Email, $ph_number);
                                    mysqli_stmt_execute($result);

                                    echo "The selected Fingerprint has been updated!";
                                    exit();
                                }
                            }
                            else{
                                if (!empty($ph_number)) {
                                    $sql="UPDATE teachers SET email=?, phone_number=? WHERE fingerprint_select=1";
                                    $result = mysqli_stmt_init($conn);
                                    if (!mysqli_stmt_prepare($result, $sql)) {
                                        echo "SQL_Error_select_Fingerprint5";
                                        exit();
                                    }
                                    else{
                                        mysqli_stmt_bind_param($result, "sd", $Email, $ph_number );
                                        mysqli_stmt_execute($result);

                                        echo "The selected Fingerprint has been updated!";
                                        exit();
                                    }
                                }
                                else{
                                    echo "The Teacher's contact info is incomplete!";
                                    exit();
                                }    
                            }  
                        }
                        else {
                            echo "The serial number is already taken!";
                            exit();
                        }
                    }
                }
            }    
        }
        else {
            echo "There's no selected Fingerprint to update!";
            exit();
        }
    }
}
// delete user 
if (isset($_POST['delete'])) {

    $sql = "SELECT fingerprint_select FROM teachers WHERE fingerprint_select=1";
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select";
        exit();
    }
    else{
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)) {
            $sql="UPDATE teachers SET del_fingerid=1 WHERE fingerprint_select=1";
            $result = mysqli_stmt_init($conn);
            if (!mysqli_stmt_prepare($result, $sql)) {
                echo "SQL_Error_delete";
                exit();
            }
            else{
                mysqli_stmt_execute($result);
                echo "Fingerprint has been deleted";
                exit();
            }
        }
        else{
            echo "Select a Fingerprint to remove";
            exit();
        }
    }
}
?>
