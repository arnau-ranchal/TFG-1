#!/usr/bin/env python3
import argparse
from decimal import Decimal
import boto3

def main():
    parser = argparse.ArgumentParser(description="Add data to a DynamoDB table with a composite key.")
    parser.add_argument("--table", default="Exponents", help="Name of the DynamoDB table.")
    parser.add_argument("--constellationM", type=str, required=True, help="Value for constellationM (partition key).")
    parser.add_argument("--simulationId", type=str, required=True, help="Value for simulationId (sort key).")
    parser.add_argument("--nodesN", type=str, required=True, help="Value for nodesN.")
    parser.add_argument("--SNR", type=str, required=True, help="Value for signalNoiseRatio.")
    parser.add_argument("--transmissionRate", type=str, required=True, help="Value for transmissionRate.")
    parser.add_argument("--errorExponent", type=str, required=True, help="Value for errorExponent.")
    parser.add_argument("--optimalRho", type=str, required=True, help="Value for optimalRho.")
    args = parser.parse_args()

    # Create a DynamoDB resource. (Make sure your AWS credentials are set.)
    dynamodb = boto3.resource('dynamodb', region_name='eu-north-1')
    table = dynamodb.Table(args.table)

    # Construct the item. Note:
    # - Numeric attributes are converted to Decimal.
    # - simulationId is kept as a string.
    item = {
        "constellationM": Decimal(args.constellationM),
        "simulationId": args.simulationId,
        "nodesN": Decimal(args.nodesN),
        "signalNoiseRatio": Decimal(args.SNR),
        "transmissionRate": Decimal(args.transmissionRate),
        "errorExponent": Decimal(args.errorExponent),
        "optimalRho": Decimal(args.optimalRho)
    }

    # Insert the item into the table
    table.put_item(Item=item)
    print(f"Data inserted into table '{args.table}' with constellationM={args.constellationM} and simulationId={args.simulationId}.")

if __name__ == "__main__":
    main()