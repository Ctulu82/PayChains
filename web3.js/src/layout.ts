import {Buffer} from 'buffer';
import * as BufferLayout from '@paychains/buffer-layout';

/**
 * Layout for a public key
 */
export const publicKey = (
  property: string = 'publicKey',
): BufferLayout.Layout => {
  return BufferLayout.blob(32, property);
};

/**
 * Layout for a 64bit unsigned value
 */
export const uint64 = (property: string = 'uint64'): BufferLayout.Layout => {
  return BufferLayout.blob(8, property);
};

/**
 * Layout for a Rust String type
 */
export const rustString = (property: string = 'string') => {
  const rsl = BufferLayout.struct(
    [
      BufferLayout.u32('length'),
      BufferLayout.u32('lengthPadding'),
      BufferLayout.blob(BufferLayout.offset(BufferLayout.u32(), -8), 'chars'),
    ],
    property,
  );
  const _decode = rsl.decode.bind(rsl);
  const _encode = rsl.encode.bind(rsl);

  rsl.decode = (buffer: any, offset: any) => {
    const data = _decode(buffer, offset);
    return data['chars'].toString('utf8');
  };

  rsl.encode = (str: any, buffer: any, offset: any) => {
    const data = {
      chars: Buffer.from(str, 'utf8'),
    };
    return _encode(data, buffer, offset);
  };

  (rsl as any).alloc = (str: any) => {
    return (
      BufferLayout.u32().span +
      BufferLayout.u32().span +
      Buffer.from(str, 'utf8').length
    );
  };

  return rsl;
};

/**
 * Layout for an Authorized object
 */
export const authorized = (property: string = 'authorized') => {
  return BufferLayout.struct(
    [publicKey('staker'), publicKey('withdrawer')],
    property,
  );
};

/**
 * Layout for a Lockup object
 */
export const lockup = (property: string = 'lockup') => {
  return BufferLayout.struct(
    [
      BufferLayout.ns64('unixTimestamp'),
      BufferLayout.ns64('epoch'),
      publicKey('custodian'),
    ],
    property,
  );
};

export function getAlloc(type: any, fields: any): number {
  let alloc = 0;
  type.layout.fields.forEach((item: any) => {
    if (item.span >= 0) {
      alloc += item.span;
    } else if (typeof item.alloc === 'function') {
      alloc += item.alloc(fields[item.property]);
    }
  });
  return alloc;
}
