#pragma once

#include "CLI/CLI.hpp"

#include "aws/core/Aws.h"
#include "aws/core/auth/AWSCredentialsProviderChain.h"
#include "aws/core/auth/signer/AWSAuthV4Signer.h"
#include "aws/core/client/ClientConfiguration.h"
#include "aws/core/http/HttpClient.h"
#include "aws/sts/STSClient.h"
#include "aws/sts/model/AssumeRoleRequest.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <memory>
#include <string>
#include <vector>
