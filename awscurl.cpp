#include "pch.h"

#include "HeaderValue.h"

// https://github.com/aws/aws-sdk-cpp
// https://github.com/aws/aws-sdk-cpp/tree/main/src/aws-cpp-sdk-core/source
// https://github.com/awsdocs/aws-doc-sdk-examples/tree/master/cpp
// 
// https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/basic-use.html
// https://sdk.amazonaws.com/cpp/api/LATEST/root/html/index.html
// (not much on this page, select "Modules" to see all of the modules)
//
// For the aws-cpp-sdk-core module (see pch.h, this is used throughout):
// https://sdk.amazonaws.com/cpp/api/LATEST/aws-cpp-sdk-core/html/annotated.html
//
// https://stackoverflow.com/questions/40457692/aws-sdk-cpp-how-to-use-curlhttpclient
//
// Base class of credential providers (AWSCredentialsProvider):
// https://sdk.amazonaws.com/cpp/api/LATEST/aws-cpp-sdk-core/html/class_aws_1_1_auth_1_1_a_w_s_credentials_provider.html
// (we use DefaultAWSCredentialsProviderChain, ProfileConfigFileAWSCredentialsProvider and SimpleAWSCredentialsProvider)
//
// STS module is used for assuming a role:
// https://sdk.amazonaws.com/cpp/api/LATEST/aws-cpp-sdk-sts/html/annotated.html

Aws::Http::URI FixupUrl(const std::string &url);

int main(int argc, char *argv[])
{
    // The --request/-X option is "transformed" via this map
    // I found out about how to handle enums from the CLI11 README.md "enum" example
    // https://github.com/CLIUtils/CLI11#examples
    // (The readme is the best documentation for the library)
    auto methodMap = std::map<std::string, Aws::Http::HttpMethod>
    {
        { "GET", Aws::Http::HttpMethod::HTTP_GET },
        { "POST", Aws::Http::HttpMethod::HTTP_POST },
        { "DELETE", Aws::Http::HttpMethod::HTTP_DELETE },
        { "PUT", Aws::Http::HttpMethod::HTTP_PUT },
        { "HEAD", Aws::Http::HttpMethod::HTTP_HEAD },
        { "PATCH", Aws::Http::HttpMethod::HTTP_PATCH }
    };

    std::optional<Aws::Http::HttpMethod> requestMethod;
    std::optional<std::string> postData;
    bool postDataBinary = false;
    std::vector<HeaderValue> headerValues{ {"Accept", "application/json"}, {"Content-Type", "application/json"} };
    bool insecureSsl = false;
    bool outputResponseHeaders = false;
    std::string profile;
    std::string region;
    std::string service("execute-api");
    std::string url;
    CLI::App app("Make AWS signature V4 http requests", "awscurl");
    try
    {
        app.add_option("-X,--request", requestMethod, "Specify request method to use (default: GET)")
            ->transform(CLI::CheckedTransformer(methodMap, CLI::ignore_case));
        app.add_option("-d,--data", postData, "Data to POST, @file to read from 'file'")
            ->option_text("<data>");
        app.add_flag("--data-binary", postDataBinary, "Read POST data file as binary, keep line breaks");
        app.add_option("-H,--header", headerValues, "Request header to add (\"<header>: <value>\")")
            ->option_text("<header> ...");
        app.add_flag("-i,--include", outputResponseHeaders, "Include response headers in output");
        app.add_flag("-k,--insecure", insecureSsl, "Allow insecure server connections when using https");
        app.add_option("--profile", profile, "AWS profile")
            ->option_text("<profile>");
        app.add_option("--region", region, "AWS region (default: profile from client config)")
            ->option_text("<region>");
        app.add_option("--service", service, "AWS service (default: \"execute-api\")")
            ->option_text("<service>");
        app.add_option("url", url, "URL to make request to")
            ->option_text("(required)")
            ->required();

        app.parse(argc, argv);
    }
    catch(CLI::ParseError &e)
    {
        return app.exit(e);
    }
    if(!requestMethod)
    {
        requestMethod.emplace(postData
                                  ? Aws::Http::HttpMethod::HTTP_POST
                                  : Aws::Http::HttpMethod::HTTP_GET);
    }
#ifdef VERBOSE_LOGGING
    std::cout << "url: " << url << std::endl;
    std::cout << "requestMethod: " << (int)*requestMethod << " [" <<
        // "reverse" lookup in methodMap
        std::find_if(methodMap.cbegin(), methodMap.cend(), [&requestMethod](const auto &pair) { return pair.second == *requestMethod; })->first << "]\n";
    std::cout << "data: <" << (postData ? "present" : "not-present") << ">\n";
    for(const auto &headerValue : headerValues)
        std::cout << "headerValue: name=" << headerValue.first << ", value=" << headerValue.second << std::endl;
#endif

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    const char *allocTag = "awscurl";

    std::unique_ptr<Aws::Client::ClientConfiguration> pClientConfig;
    std::shared_ptr<Aws::Auth::AWSCredentialsProvider> spCredentialsProvider;
    std::string roleArn;
    if(profile.length() == 0)
    {
        pClientConfig = std::make_unique<Aws::Client::ClientConfiguration>();
        spCredentialsProvider = Aws::MakeShared<Aws::Auth::DefaultAWSCredentialsProviderChain>(allocTag);
    }
    else
    {
        pClientConfig = std::make_unique<Aws::Client::ClientConfiguration>(profile.c_str());
        // Default is to load credentials from this profile
        auto credentialsProfile = profile;
        // Load the .aws/config file to see if this profile is configured to assume another role
        Aws::Config::AWSConfigFileProfileConfigLoader configLoader(Aws::Auth::GetConfigProfileFilename(), true);
        if(configLoader.Load() && configLoader.GetProfiles().find(profile) != configLoader.GetProfiles().cend())
        {
            const Aws::Config::Profile &awsProfile = configLoader.GetProfiles().at(profile);
            if(awsProfile.GetSourceProfile().length() != 0 && awsProfile.GetRoleArn().length() != 0)
            {
                // We need to assume the role configured in the profile, we will do this by creating a
                // credential provider using the source profile and set the role ARN to be assumed
                //
                // Implementing how I read the this link: https://docs.aws.amazon.com/cli/latest/topic/config-vars.html
                // SourceProfile is only used to assume the RoleArn, not used for ClientConfiguration
                credentialsProfile = awsProfile.GetSourceProfile();
                roleArn = awsProfile.GetRoleArn();
            }
        }
        spCredentialsProvider = Aws::MakeShared<Aws::Auth::ProfileConfigFileAWSCredentialsProvider>(allocTag, credentialsProfile.c_str());
    }
#ifdef VERBOSE_LOGGING
    std::cout << "profile: " << profile << std::endl;
    std::cout << "roleArn: " << roleArn << std::endl;
    std::cout << "pClientConfig->region: " << pClientConfig->region << std::endl;
#endif
    if(region.length() == 0)
    {
        region = pClientConfig->region;
    }

    if(roleArn.length() != 0)
    {
        Aws::STS::Model::AssumeRoleRequest assumeRoleRequest;
        assumeRoleRequest
            .WithRoleSessionName("awscurl")
            .WithRoleArn(roleArn)
            .SetDurationSeconds(900);

        Aws::STS::STSClient stsClient(spCredentialsProvider, *pClientConfig);
        auto assumeRoleOutcome = stsClient.AssumeRole(assumeRoleRequest);
        if(!assumeRoleOutcome.IsSuccess())
        {
            std::cerr << "AssumeRole() failed:\n";
            const auto &error = assumeRoleOutcome.GetError();
            std::cerr << error.GetMessage() << std::endl;
            return 1;
        }

        // "Overwrite" spCredentialsProvider with one constructed with the assumed credentials
        const auto &credentials = assumeRoleOutcome.GetResult().GetCredentials();
        spCredentialsProvider = Aws::MakeShared<Aws::Auth::SimpleAWSCredentialsProvider>(
            allocTag,
            credentials.GetAccessKeyId(),
            credentials.GetSecretAccessKey(),
            credentials.GetSessionToken());
    }

    auto spAuthV4Signer = Aws::MakeShared<Aws::Client::AWSAuthV4Signer>(
        allocTag,
        spCredentialsProvider,
        service.c_str(),
        region);

    auto spHttpRequest = Aws::Http::CreateHttpRequest(
        FixupUrl(url),
        *requestMethod,
        Aws::IOStreamFactory(Aws::Utils::Stream::DefaultResponseStreamFactoryMethod));
    for(const auto &headerValue : headerValues)
    {
        spHttpRequest->SetHeaderValue(headerValue.first, headerValue.second);
    }
    if(insecureSsl)
    {
        pClientConfig->verifySSL = false;
    }

    if(postData)
    {
        std::shared_ptr<Aws::IOStream> spBodyStream;
        if(postData->front() == '@')
        {
            auto fileName = postData->substr(1);
            Aws::FStream inputStream(fileName, postDataBinary ? std::ios::in | std::ios::binary : std::ios::in);
            if(!inputStream.is_open())
            {
                std::cerr << "Error opening input file '" << fileName << "'\n";
                return 1;
            }

            if(postDataBinary)
            {
                spBodyStream = Aws::MakeShared<Aws::FStream>(allocTag, std::move(inputStream));
            }
            else
            {
                spBodyStream = Aws::MakeShared<Aws::StringStream>(allocTag);
                // This follows curl's behavior when reading the data file
                // "carriage returns, newlines and null bytes are stripped out"
                std::string line;
                while(std::getline(inputStream, line))
                {
                    *spBodyStream << line;
                }
            }
        }
        else
        {
            spBodyStream = Aws::MakeShared<Aws::StringStream>(allocTag, *postData);
        }
        spBodyStream->seekp(0, std::ios::end);
        auto bodyLength = spBodyStream->tellp();

        spHttpRequest->AddContentBody(spBodyStream);
        // Not required to call SetContentType() as long as headerValues contains "Content-Type"
        Aws::StringStream intConverter;
        intConverter << bodyLength;
        spHttpRequest->SetContentLength(intConverter.str());
#ifdef VERBOSE_LOGGING
        std::cout << "Added content body, length = " << bodyLength << std::endl;
#endif
    }

    spAuthV4Signer->SignRequest(*spHttpRequest);

    // Two ways of creating the HttpClient, both worked in my testing on Windows
    // I assume CreateHttpClient() always does the right thing...
    // (Non-Windows platforms should have Aws::Http::CurlHttpClient (from curl/CurlHttpClient.h))
    //typedef Aws::Http::WinHttpSyncHttpClient httpClient_t;
    //httpClient_t httpClient(clientConfig);
    auto spHttpClient = Aws::Http::CreateHttpClient(*pClientConfig);

    auto spHttpResponse = spHttpClient->MakeRequest(spHttpRequest);
    std::cout << "response code: " << spHttpResponse->GetResponseCode() << std::endl;
    if(outputResponseHeaders)
    {
        for(const auto &headerValue : spHttpResponse->GetHeaders())
        {
            std::cout << headerValue.first << ": " << headerValue.second << std::endl;
        }
    }

    // Looking at the AWS HttpClient source, the response body is fully in memory
    // when MakeRequest() completes, so rdbuf() will return the entire body
    std::cout << spHttpResponse->GetResponseBody().rdbuf();

    Aws::ShutdownAPI(options);
    return 0;
}

// Fixup the URL by URL encoding the query parameters
Aws::Http::URI FixupUrl(const std::string &url)
{
    Aws::Http::URI uri(url);
    // Copy to a new object where we will build the new URL, start by clearing out the query parameters
    Aws::Http::URI newUri(uri);
    newUri.SetQueryString("");

    // Adding back the query parameters in this way will encode them correctly
    for(const auto &queryParam : uri.GetQueryStringParameters())
    {
        newUri.AddQueryStringParameter(queryParam.first.c_str(), queryParam.second);
    }
    return newUri;
}
