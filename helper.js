function deviceRoleWrap(role) {
    switch (role) {
    case VecsDevice.RoleUndefined:
        return "Undefined";
    case VecsDevice.RoleDoctor:
        return "Doctor";
    case VecsDevice.RolePatientHand:
        return "Patient (on hand)";
    case VecsDevice.RolePatientBack:
        return "Patient (on back)";
    }
}

function gyroRangeWrap(range) {
    var a = [250, 500, 1000, 2000];
    return a[range];
}

function accelRangeWrap(range) {
    return Math.pow(2, range + 1);
}
